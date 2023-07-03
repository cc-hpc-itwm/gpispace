// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhgcom/connection.hpp>

#include <fhgcom/peer.hpp>

#include <fhg/assert.hpp>
#include <util-generic/functor_visitor.hpp>

#include <boost/asio/read.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <stdexcept>
#include <utility>
#include <vector>

namespace fhg
{
  namespace com
  {
    namespace
    {
      template <typename Arg0, typename Arg2, typename... Buffer>
        void async_read_wrapper
          ( Arg0 const& arg0
          , Arg2 const& arg2
          , Buffer&&... buffer
          )
      {
        return fhg::util::visit<void>
          ( arg0
          , [&] (auto const& socket)
            {
              return async_read
                ( *socket
                , std::vector<::boost::asio::mutable_buffer>
                    {::boost::asio::buffer (std::forward<Buffer> (buffer)...)}
                , arg2
                );
            }
          );
      }
    }

    template<typename SocketPtr>
      tcp_socket_t& raw_socket (SocketPtr& socket)
    {
      struct : ::boost::static_visitor<tcp_socket_t&>
      {
        tcp_socket_t& operator() (std::unique_ptr<tcp_socket_t>& stream) const
        {
          return *stream;
        }
        tcp_socket_t& operator() (std::unique_ptr<ssl_stream_t>& stream) const
        {
          return stream->next_layer();
        }
      } visitor;
      return ::boost::apply_visitor (visitor, socket);
    }

    connection_t::connection_t
      ( ::boost::asio::io_service & io_service
      , ::boost::asio::ssl::context* ctx
      , ::boost::asio::io_service::strand const& strand
      , std::function<void (ptr_t connection, std::unique_ptr<message_t>)> handle_hello_message
      , std::function<void (ptr_t connection, std::unique_ptr<message_t>)> handle_user_data
      , std::function<void (ptr_t connection, ::boost::system::error_code const&)> handle_error
      , peer_t* peer
      )
      : _peer (peer)
      , strand_(strand)
      , socket_
        ( [&]() -> socket_t
          {
            if (ctx)
            {
              return std::make_unique<ssl_stream_t> (io_service, *ctx);
            }

            return std::make_unique<tcp_socket_t> (io_service);
          }()
        )
      , _raw_socket (raw_socket (socket_))
      , _handle_hello_message (handle_hello_message)
      , _handle_user_data (handle_user_data)
      , _handle_error (handle_error)
    {}

    connection_t::~connection_t ()
    {
        stop();

      // Do not close before all async operations are done. This
      // includes things posted onto the strand without mentioning the
      // socket, which are *not* canceled when _socket.cancel() is
      // called and would likely segfault. By only closing in the
      // dtor, those operations will see a shutdown, but not closed
      // socket, and after failing to do whatever they want to do
      // release their reference to connection_t one after the other.
      ::boost::system::error_code ignore;
      _raw_socket.close (ignore);
    }

    ::boost::asio::ip::tcp::socket & connection_t::socket()
    {
      return _raw_socket;
    }

    void connection_t::start()
    {
      start_read();
    }

    void connection_t::stop ()
    {
      ::boost::system::error_code ignore;

      _raw_socket.cancel (ignore);
      _raw_socket.shutdown
        (::boost::asio::ip::tcp::socket::shutdown_both, ignore);

      fhg::util::visit<void>
        ( socket_
        , [&] (std::unique_ptr<ssl_stream_t> const& stream)
          {
            stream->shutdown (ignore);
          }
        , [] (std::unique_ptr<tcp_socket_t> const&) {}
        );
    }

    void connection_t::request_handshake
      (std::shared_ptr<std::promise<std::exception_ptr>> connect_done)
    {
      auto connection (shared_from_this());

      auto const call_response
        ( [=] (::boost::system::error_code const& ec)
          {
            connection->_peer->request_handshake_response
              (connection->remote_address(), connect_done, ec);
          }
        );

      fhg::util::visit<void>
        ( socket_
        , [=] (std::unique_ptr<ssl_stream_t> const& stream)
          {
            stream->async_handshake ( ::boost::asio::ssl::stream_base::client
                                    , strand_.wrap (call_response)
                                    );
          }
        , [=] (std::unique_ptr<tcp_socket_t> const&)
          {
            strand_.post ([=] { call_response ({}); });
          }
        );
    }

    void connection_t::acknowledge_handshake()
    {
      auto connection (shared_from_this());

      auto const call_response
        ( [=] (::boost::system::error_code const& ec)
          {
            connection->_peer->acknowledge_handshake_response (connection, ec);
          }
        );

      fhg::util::visit<void>
        ( socket_
        , [=] (std::unique_ptr<ssl_stream_t> const& stream)
          {
            stream->async_handshake ( ::boost::asio::ssl::stream_base::server
                                    , strand_.wrap (call_response)
                                    );
          }
        , [=] (std::unique_ptr<tcp_socket_t> const&)
          {
            strand_.post ([=] { call_response ({}); });
          }
        );
    }

    void connection_t::start_read ()
    {
      in_message_ = std::make_unique<message_t>
        (this->remote_address(), this->local_address());

      async_read_wrapper
      	( socket_
        , strand_.wrap
            ( std::bind ( &connection_t::handle_read_header
                        , shared_from_this()
                        , std::placeholders::_1
                        , std::placeholders::_2
                        )
        	)
      	, &in_message_->header, sizeof (p2p::header_t)
      	);
    }

    void connection_t::handle_read_header( ::boost::system::error_code const& ec
                                         , std::size_t bytes_transferred
                                         )
    {
      if (! ec && bytes_transferred > 0)
      {
        fhg_assert (bytes_transferred == sizeof (p2p::header_t));

        // WORK HERE: convert for local endianess!
        in_message_->resize ();

        async_read_wrapper
          ( socket_
          , strand_.wrap
              ( std::bind ( &connection_t::handle_read_data
                          , shared_from_this()
                          , std::placeholders::_1
                          , std::placeholders::_2
                          )
          	  )
          , in_message_->data
          );
      }
      else
      {
        in_message_.reset();
        _handle_error (shared_from_this(), ec);
      }
    }

    void connection_t::handle_read_data( ::boost::system::error_code const& ec
                                       , std::size_t
                                       )
    {
      if (! ec)
      {
        if (in_message_->is_hello())
        {
          _handle_hello_message (shared_from_this(), std::move (in_message_));
        }
        else
        {
          _handle_user_data (shared_from_this(), std::move (in_message_));
        }

        start_read ();
      }
      else
      {
        in_message_.reset();
        _handle_error (shared_from_this(), ec);
      }
    }

    void connection_t::start_send (message_t const& message)
    {
      return fhg::util::visit<void>
        ( socket_
        , [&] (auto const& socket)
          {
            return async_write
              ( *socket
              , std::vector<::boost::asio::const_buffer>
                { ::boost::asio::buffer (&message.header, sizeof (p2p::header_t))
                , ::boost::asio::buffer (message.data)
                }
              , strand_.wrap
                  ( std::bind ( &connection_t::handle_write
                              , shared_from_this()
                              , std::placeholders::_1
                              )
                  )
              );
          }
        );
    }

    void connection_t::handle_write (::boost::system::error_code const& ec)
    {
      if (! ec)
      {
        to_send_.front().handler (ec);
        to_send_.pop_front();

        if (! to_send_.empty())
        {
          start_send (to_send_.front()._message);
        }
      }
      else
      {
        _handle_error (shared_from_this(), ec);
      }
    }

    void connection_t::async_send ( message_t msg
                                  , completion_handler_t hdl
                                  )
    {
      ::boost::shared_ptr<connection_t> that (shared_from_this());
      strand_.post
        ( [that, msg, hdl]
          {
            auto& send_queue {that->to_send_};
            send_queue.emplace_back (std::move (msg), std::move (hdl));
            if (send_queue.size() == 1)
            {
              that->start_send (send_queue.front()._message);
            }
          }
        );
    }

    connection_t::to_send_t::to_send_t
        (message_t message, connection_t::completion_handler_t hdl)
      : handler (std::move (hdl))
      , _message (std::move (message))
    {}
  }
}
