#include <fhglog/minimal.hpp>

#include "connection.hpp"

#include <fhgcom/util/to_hex.hpp>

using namespace boost::asio::ip;

namespace fhg
{
  namespace com
  {
    connection_t::connection_t( boost::asio::io_service & io_service
                              )
      : strand_(io_service)
      , socket_(io_service)
      , deadline_(io_service)
      , message_handler_(0)
    {}

    connection_t::~connection_t ()
    {
      DLOG(TRACE, "connection destroyed");

      if (in_data_.msg)
      {
        delete in_data_.msg;
        in_data_.msg = 0;
      }
    }

    message_handler_t * connection_t::set_message_handler (message_handler_t * h)
    {
      message_handler_t * old = message_handler_;
      message_handler_ = h;
      return old;
    }

    tcp::socket & connection_t::socket()
    {
      return socket_;
    }

    void connection_t::start()
    {
      start_read();
    }

    void connection_t::async_send ( const message_t * msg
                                  , completion_handler_t hdl
                                  , boost::posix_time::time_duration timeout
                                  )
    {
      strand_.post (boost::bind( &self::start_send
                               , this
                               , out_data_t (msg, hdl, timeout)
                               ));
    }

    void connection_t::handle_read_header( const boost::system::error_code & ec
                                         , std::size_t bytes_transferred
                                         )
    {
      if (! ec)
      {
        assert (bytes_transferred == header_length);

        std::size_t inbound_data_size = 0;

        {
          std::istringstream is (std::string (in_data_.header, header_length));
          if (! (is >> std::hex >> inbound_data_size))
          {
            LOG(ERROR, "could not parse header: " << is.str());
            // TODO: call handler
            if (message_handler_)
            {
              message_handler_->handle_error
                (boost::system::error_code
                ( boost::system::errc::invalid_argument
                , boost::system::generic_category()
                )
                );
            }
          }
          return;
        }

        DLOG(DEBUG, "going to receive " << inbound_data_size << " bytes");
        LOG_IF( WARN
              , inbound_data_size > (1<<22)
              , "incoming message is quite large (>4MB)!"
              );

        in_data_.msg = new message_t (inbound_data_size);
        boost::asio::async_read ( socket_
                                , boost::asio::buffer (in_data_.msg->data())
                                , strand_.wrap
                                ( boost::bind ( &self::handle_read_data
                                              , this
                                              , boost::asio::placeholders::error
                                              , boost::asio::placeholders::bytes_transferred
                                              )
                                ));
      }
      else
      {
        if (message_handler_) message_handler_->handle_error (ec);
      }
    }

    void connection_t::handle_read_data( const boost::system::error_code & ec
                                       , std::size_t bytes_transferred
                                       )
    {
      if (! ec)
      {
        message_t * m = in_data_.msg;
        in_data_.msg = 0;
        if (message_handler_)
          message_handler_->handle_recv ( m );
        else
        {
          // drop message
          delete m;
        }

        start_read ();
      }
      else
      {
        if (message_handler_)
          message_handler_->handle_error (ec);
      }
    }

    void connection_t::start_send ()
    {
      assert (to_send_.size() > 0);

      const out_data_t & d (to_send_.front());

      DLOG( TRACE
          , "initiating write of " << d.msg->size() << " bytes "
          << util::basic_hex_converter<128>::convert(d.msg->data())
          );

      boost::asio::async_write( socket_
                              , d.to_buffers()
                              , strand_.wrap (boost::bind( &self::handle_write
                                                         , this
                                                         , boost::asio::placeholders::error
                                                         )
                                             )
                              );
    }

    void connection_t::start_send (const connection_t::out_data_t & d)
    {
      bool send_in_progress = !to_send_.empty();
      to_send_.push_back (d);
      if (! send_in_progress)
      {
        start_send();
      }
    }

    void connection_t::handle_write (const boost::system::error_code & ec)
    {
      if (! ec)
      {
        DLOG(TRACE, "write completed");
        out_data_t d (to_send_.front());
        to_send_.pop_front();

        try
        {
          if (d.hdl)
            d.hdl (ec);
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "completion handler failed: " << ex.what());
        }

        if (d.msg)
        {
          delete d.msg; d.msg = 0;
        }

        if (! to_send_.empty())
          start_send();
      }
      else
      {
        if (message_handler_)
          message_handler_->handle_error (ec);
      }
    }

    void connection_t::start_read ()
    {
      boost::asio::async_read( socket_
                             , boost::asio::buffer(in_data_.header, header_length)
                             , strand_.wrap
                             ( boost::bind ( &self::handle_read_header
                                           , this
                                           , boost::asio::placeholders::error
                                           , boost::asio::placeholders::bytes_transferred
                                           )
                             ));
    }
  }
}
