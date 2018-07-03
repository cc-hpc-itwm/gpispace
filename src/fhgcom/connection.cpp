
#include <fhgcom/connection.hpp>

#include <fhg/assert.hpp>

#include <util-generic/cxx14/make_unique.hpp>

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
      , std::unique_ptr<boost::asio::ssl::context> const& ctx
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
      , std::unique_ptr<boost::asio::ssl::context> const& ctx
      , boost::asio::io_service::strand const& strand
      , std::function<void (ptr_t connection, const message_t*)> handle_hello_message
      , std::function<void (ptr_t connection, const message_t*)> handle_user_data
      , std::function<void (ptr_t connection, const boost::system::error_code&)> handle_error
      )
      : strand_(strand)
      , socket_ (make_socket (io_service, ctx))
      , _raw_socket (raw_socket (socket_))
      , _handle_hello_message (handle_hello_message)
      , _handle_user_data (handle_user_data)
      , _handle_error (handle_error)
      , in_message_(new message_t)
    {}

    connection_t::~connection_t ()
    {
        stop();

      if (in_message_)
      {
        delete in_message_;
        in_message_ = nullptr;
      }
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
      boost::system::error_code ec;
      if (socket().is_open())
      {
        socket().shutdown (boost::asio::ip::tcp::socket::shutdown_both, ec);
      }
      socket().close(ec);
    }

    void connection_t::request_handshake()
    {
      struct : boost::static_visitor<void>
      {
        void operator() (std::unique_ptr<ssl_stream_t>& stream) const
        {
          stream->handshake (boost::asio::ssl::stream_base::client);
        }
        void operator() (std::unique_ptr<tcp_socket_t>&) const
        {
        }
      } visitor;
      boost::apply_visitor (visitor, socket_);
    }

    void connection_t::acknowledge_handshake
      (std::function <void (const boost::system::error_code&)> handler)
    {
      struct visitor_t : boost::static_visitor<void>
      {
        void operator() (std::unique_ptr<ssl_stream_t>& stream) const
        {
          stream->async_handshake
            (boost::asio::ssl::stream_base::server, _handler);
        }
        void operator() (std::unique_ptr<tcp_socket_t>&) const
        {
          _handler ({});
        }
        std::function<void (const boost::system::error_code&)> _handler;
        visitor_t (decltype (_handler) handler) : _handler (handler) {}
      } visitor = {strand_.wrap (handler)};
      boost::apply_visitor (visitor, socket_);
    }

    void connection_t::start_read ()
    {
      fhg_assert (in_message_ != nullptr);

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
        in_message_->resize(0);
        _handle_error (shared_from_this(), ec);
      }
    }

    void connection_t::handle_read_data( const boost::system::error_code & ec
                                       , std::size_t
                                       )
    {
      if (! ec)
      {
        message_t * m = in_message_;
        in_message_ = nullptr;

            if (m->header.type_of_msg == p2p::HELLO_PACKET)
            {
              _handle_hello_message (shared_from_this(), m);
            }
            else
            {
              _handle_user_data (shared_from_this(), m);
            }

            in_message_ = new message_t;

        start_read ();
      }
      else
      {
        in_message_->resize(0);
            _handle_error (shared_from_this(), ec);
      }
    }

    void connection_t::start_send (connection_t::to_send_t const & s)
    {
      bool const send_in_progress = !to_send_.empty();
      to_send_.push_back (s);
      if (! send_in_progress)
        start_send();
    }

    void connection_t::start_send ()
    {
      fhg_assert (to_send_.size() > 0);

      if (to_send_.empty ())
        return;

      const to_send_t & d (to_send_.front());

      try
      {
        async_write_wrapper
          ( socket_
          , d.to_buffers()
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
        to_send_t const d (to_send_.front());
        to_send_.pop_front();

        if (d.handler)
        {
            d.handler (ec);
        }

        if (! to_send_.empty())
          start_send ();
      }
      else
      {
        _handle_error (shared_from_this(), ec);
      }
    }

    void connection_t::async_send ( const message_t * msg
                                  , completion_handler_t hdl
                                  )
    {
      boost::shared_ptr<connection_t> that (shared_from_this());
      strand_.post
        ([that, msg, hdl] { that->start_send (to_send_t (msg, hdl)); });
    }
  }
}
