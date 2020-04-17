#include <fhgcom/connection.hpp>

#include <fhgcom/peer.hpp>

#include <fhg/assert.hpp>

#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/functor_visitor.hpp>

#include <boost/asio/ssl/context.hpp>
#include <boost/system/error_code.hpp>

#include <functional>

namespace fhg
{
  namespace com
  {
    namespace
    {
      struct async_read_visitor : public boost::static_visitor<>
      {
        template<typename Arg0, typename Arg1, typename Arg2>
          void operator() (Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2) const
        {
          async_read (*arg0, arg1, arg2);
        }
      };

      template <typename Arg0, typename Arg1, typename Arg2>
        void async_read_wrapper (Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2)
      {
        boost::apply_visitor
          ( std::bind ( async_read_visitor()
                      , std::placeholders::_1
                      , std::ref (arg1)
                      , std::ref (arg2)
                      )
          , arg0
          );
      }

      struct async_write_visitor : public boost::static_visitor<>
      {
        template<typename Arg0, typename Arg1, typename Arg2>
          void operator() (Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2) const
        {
          async_write (*arg0, arg1, arg2);
        }
      };

      template <typename Arg0, typename Arg1, typename Arg2>
        void async_write_wrapper (Arg0 const& arg0, Arg1 const& arg1, Arg2 const& arg2)
      {
        boost::apply_visitor
          ( std::bind ( async_write_visitor()
                      , std::placeholders::_1
                      , std::ref (arg1)
                      , std::ref (arg2)
                      )
          , arg0
          );
      }
    }

    socket_t make_socket
      ( boost::asio::io_service& io_service
      , boost::asio::ssl::context* ctx
      )
    {
      if (ctx)
      {
        return fhg::util::cxx14::make_unique<ssl_stream_t> (io_service, *ctx);
      }
      else
      {
        return fhg::util::cxx14::make_unique<tcp_socket_t> (io_service);
      }
    }
    template<typename SocketPtr>
      tcp_socket_t& raw_socket (SocketPtr& socket)
    {
      struct : boost::static_visitor<tcp_socket_t&>
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
      return boost::apply_visitor (visitor, socket);
    }

    connection_t::connection_t
      ( boost::asio::io_service & io_service
      , boost::asio::ssl::context* ctx
      , boost::asio::io_service::strand const& strand
      , std::function<void (ptr_t connection, std::unique_ptr<message_t>)> handle_hello_message
      , std::function<void (ptr_t connection, std::unique_ptr<message_t>)> handle_user_data
      , std::function<void (ptr_t connection, const boost::system::error_code&)> handle_error
      , peer_t* peer
      )
      : _peer (peer)
      , strand_(strand)
      , socket_ (make_socket (io_service, ctx))
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
      boost::system::error_code ignore;
      _raw_socket.close (ignore);
    }

    boost::asio::ip::tcp::socket & connection_t::socket()
    {
      return _raw_socket;
    }

    void connection_t::start()
    {
      start_read();
    }

    void connection_t::stop ()
    {
      boost::system::error_code ignore;

      _raw_socket.cancel (ignore);
      _raw_socket.shutdown
        (boost::asio::ip::tcp::socket::shutdown_both, ignore);

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
      (std::shared_ptr<util::thread::event<std::exception_ptr>> connect_done)
    {
      auto connection (shared_from_this());

      auto const call_response
        ( [=] (boost::system::error_code const& ec)
          {
            connection->_peer->request_handshake_response
              (connection->remote_address(), connect_done, ec);
          }
        );

      fhg::util::visit<void>
        ( socket_
        , [=] (std::unique_ptr<ssl_stream_t> const& stream)
          {
            stream->async_handshake ( boost::asio::ssl::stream_base::client
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
        ( [=] (boost::system::error_code const& ec)
          {
            connection->_peer->acknowledge_handshake_response (connection, ec);
          }
        );

      fhg::util::visit<void>
        ( socket_
        , [=] (std::unique_ptr<ssl_stream_t> const& stream)
          {
            stream->async_handshake ( boost::asio::ssl::stream_base::server
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
      in_message_ = fhg::util::cxx14::make_unique<message_t>();

      async_read_wrapper
      	( socket_
      	, boost::asio::buffer (&in_message_->header, sizeof (p2p::header_t))
        , strand_.wrap
            ( std::bind ( &connection_t::handle_read_header
                        , shared_from_this()
                        , std::placeholders::_1
                        , std::placeholders::_2
                        )
        	)
      	);
    }

    void connection_t::handle_read_header( const boost::system::error_code & ec
                                         , std::size_t IF_FHG_ASSERT (bytes_transferred)
                                         )
    {
      if (! ec)
      {
        fhg_assert (bytes_transferred == sizeof(p2p::header_t));

        // WORK HERE: convert for local endianess!
        in_message_->resize ();

        async_read_wrapper
          ( socket_
          , boost::asio::buffer (in_message_->data)
          , strand_.wrap
              ( std::bind ( &connection_t::handle_read_data
                          , shared_from_this()
                          , std::placeholders::_1
                          , std::placeholders::_2
                          )
          	  )
          );
      }
      else
      {
        in_message_.reset();
        _handle_error (shared_from_this(), ec);
      }
    }

    void connection_t::handle_read_data( const boost::system::error_code & ec
                                       , std::size_t
                                       )
    {
      if (! ec)
      {
        auto m (std::move (in_message_));

        if (m->header.type_of_msg == p2p::HELLO_PACKET)
        {
          _handle_hello_message (shared_from_this(), std::move (m));
        }
        else
        {
          _handle_user_data (shared_from_this(), std::move (m));
        }

        start_read ();
      }
      else
      {
        in_message_.reset();
        _handle_error (shared_from_this(), ec);
      }
    }

    void connection_t::start_send ()
    {
      fhg_assert (to_send_.size() > 0);

      if (to_send_.empty ())
        return;

      try
      {
        async_write_wrapper
          ( socket_
          , to_send_.front().buffers
          , strand_.wrap
              ( std::bind ( &connection_t::handle_write
            		  	  , shared_from_this()
            		  	  , std::placeholders::_1
              	  	  	  )
              )
          );
      }
      catch (std::length_error const &)
      {
        strand_.post (std::bind( &connection_t::handle_write
                               , shared_from_this()
                               , boost::system::errc::make_error_code (boost::system::errc::bad_message)
                               ));
      }
    }

    void connection_t::handle_write (const boost::system::error_code & ec)
    {
      if (! ec)
      {
        auto const handler (std::move (to_send_.front().handler));
        to_send_.pop_front();

        if (handler)
        {
          handler (ec);
        }

        if (! to_send_.empty())
          start_send ();
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
      boost::shared_ptr<connection_t> that (shared_from_this());
      strand_.post
        ( [that, msg, hdl]
          {
            bool const send_in_progress (!that->to_send_.empty());
            that->to_send_.emplace_back (std::move (msg), std::move (hdl));
            if (!send_in_progress)
            {
              that->start_send();
            }
          }
        );
    }

    connection_t::to_send_t::to_send_t
        (message_t message, connection_t::completion_handler_t hdl)
      : handler (std::move (hdl))
      , _message (std::move (message))
      , buffers()
    {
      if (_message.data.size () != _message.header.length)
      {
        throw std::length_error ("header/data length mismatch");
      }

      buffers.reserve (2);
      buffers.push_back
        (boost::asio::buffer (&_message.header, sizeof (p2p::header_t)));
      buffers.push_back (boost::asio::buffer (_message.data));
    }
  }
}
