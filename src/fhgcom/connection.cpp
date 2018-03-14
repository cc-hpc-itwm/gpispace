
#include <fhgcom/connection.hpp>

#include <fhg/assert.hpp>

#include <util-generic/cxx14/make_unique.hpp>

#include <boost/system/error_code.hpp>

#include <functional>

namespace fhg
{
  namespace com
  {
    connection_t::connection_t
      ( boost::asio::io_service & io_service
      , std::unique_ptr<boost::asio::ssl::context> const& ctx
      , boost::asio::io_service::strand const& strand
      , std::function<void (ptr_t connection, const message_t*)> handle_hello_message
      , std::function<void (ptr_t connection, const message_t*)> handle_user_data
      , std::function<void (ptr_t connection, const boost::system::error_code&)> handle_error
      )
      : strand_(strand)
      , _handle_hello_message (handle_hello_message)
      , _handle_user_data (handle_user_data)
      , _handle_error (handle_error)
      , in_message_(new message_t)
      , ssl_enabled_ (ctx)
    {
      if (ctx)
      {
        socket_ = fhg::util::cxx14::make_unique<ssl_stream_t> (io_service, *ctx);
      }
      else
      {
        socket_ = fhg::util::cxx14::make_unique<tcp_socket_t> (io_service);
      }
    }

    connection_t::~connection_t ()
    {
        stop();

      if (in_message_)
      {
        delete in_message_;
        in_message_ = nullptr;
      }
    }

    tcp_socket_t& connection_t::socket_or_next_layer_socket()
    {
      if (ssl_enabled_)
      {
        return boost::get<std::unique_ptr<ssl_stream_t>> (socket_)->next_layer();
      }
      else
      {
        return *boost::get<std::unique_ptr<tcp_socket_t>> (socket_);
      }
    }

    tcp_socket_t& connection_t::get_tcp_socket()
    {
      return *boost::get<std::unique_ptr<tcp_socket_t>> (socket_);
    }

    ssl_stream_t& connection_t::get_ssl_stream()
    {
      return *boost::get<std::unique_ptr<ssl_stream_t>> (socket_);
    }

    void connection_t::start()
    {
      start_read();
    }

    void connection_t::stop ()
    {
      boost::system::error_code ec;
      if (socket_or_next_layer_socket().is_open())
      {
        socket_or_next_layer_socket().shutdown (boost::asio::ip::tcp::socket::shutdown_both, ec);
      }
      socket_or_next_layer_socket().close(ec);
    }

    void connection_t::start_read ()
    {
      fhg_assert (in_message_ != nullptr);

      auto const& handler = std::bind ( &connection_t::handle_read_header
                                      , shared_from_this()
                                      , std::placeholders::_1
                                      , std::placeholders::_2
                                      );
      if (ssl_enabled_)
      {
        boost::asio::async_read
          ( get_ssl_stream()
          , boost::asio::buffer ( &in_message_->header, sizeof(p2p::header_t))
          , strand_.wrap (handler)
          );
      }
      else
      {
        boost::asio::async_read
          ( get_tcp_socket()
          , boost::asio::buffer ( &in_message_->header, sizeof(p2p::header_t))
          , strand_.wrap (handler)
          );
      }
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

        auto const& handler = std::bind ( &connection_t::handle_read_data
                                        , shared_from_this()
                                        , std::placeholders::_1
                                        , std::placeholders::_2
                                        );

        if (ssl_enabled_)
        {
          boost::asio::async_read ( get_ssl_stream()
                                  , boost::asio::buffer (in_message_->data)
                                  , strand_.wrap (handler)
                                  );
        }
        else
        {
          boost::asio::async_read ( get_tcp_socket()
                                  , boost::asio::buffer (in_message_->data)
                                  , strand_.wrap (handler)
                                  );
        }
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
        auto const& handler = std::bind ( &connection_t::handle_write
                                        , shared_from_this()
                                        , std::placeholders::_1
                                        );

        if (ssl_enabled_)
        {
          boost::asio::async_write ( get_ssl_stream()
                                   , d.to_buffers()
                                   , strand_.wrap (handler)
                                   );
        }
        else
        {
          boost::asio::async_write ( get_tcp_socket()
                                   , d.to_buffers()
                                   , strand_.wrap (handler)
                                   );
        }
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
