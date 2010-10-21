#include <fhglog/minimal.hpp>

#include "connection.hpp"

#include <fhgcom/util/to_hex.hpp>

using namespace boost::asio::ip;

namespace fhg
{
  namespace com
  {
    connection_t::connection_t( boost::asio::io_service & io_service
                              , connection_t::handler_t * h
                              )
      : strand_(io_service)
      , socket_(io_service)
      , deadline_(io_service)
      , callback_handler_(h)
      , in_message_(new message_t)
    {}

    connection_t::~connection_t ()
    {
      set_callback_handler(0);

      DLOG(TRACE, "connection destroyed");

      if (in_message_)
      {
        delete in_message_;
        in_message_ = 0;
      }
    }

    connection_t::handler_t * connection_t::set_callback_handler (connection_t::handler_t * h)
    {
      handler_t * old = callback_handler_;
      callback_handler_ = h;
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

    void connection_t::stop ()
    {
      //      socket_.shutdown (both...);
      socket_.close();
      // callback_handler_->handle_error (this, 0);
    }

    void connection_t::start_read ()
    {
      assert (in_message_ != 0);

      boost::asio::async_read( socket_
                             , boost::asio::buffer (&in_message_->header, sizeof(message_t::header_t))
                             , strand_.wrap
                             ( boost::bind ( &self::handle_read_header
                                           , this
                                           , boost::asio::placeholders::error
                                           , boost::asio::placeholders::bytes_transferred
                                           )
                             ));
    }

    void connection_t::handle_read_header( const boost::system::error_code & ec
                                         , std::size_t bytes_transferred
                                         )
    {
      if (! ec)
      {
        assert (bytes_transferred == sizeof(message_t::header_t));

        // WORK HERE: convert for local endianess!

        DLOG(DEBUG, "going to receive " << in_message_->header.length << " bytes");
        LOG_IF( WARN
              , in_message_->header.length > (1<<22)
              , "incoming message is quite large (>4MB)!"
              );
        in_message_->resize ();
        boost::asio::async_read ( socket_
                                , boost::asio::buffer (in_message_->data)
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
        if (callback_handler_) callback_handler_->handle_error (this, ec);
      }
    }

    void connection_t::handle_read_data( const boost::system::error_code & ec
                                       , std::size_t bytes_transferred
                                       )
    {
      if (! ec)
      {
        message_t * m = in_message_;
        in_message_ = 0;

        if (callback_handler_)
        {
          try
          {
            if (p2p::type_of_message_traits::is_system_data(m->header.type_of_msg))
            {
              callback_handler_->handle_system_data (this, m);
            }
            else
            {
              callback_handler_->handle_user_data (this, m);
            }

            in_message_ = new message_t;
          }
          catch (std::exception const & ex)
          {
            LOG(ERROR, "could not pass received message to callback handler: " << ex.what());
            throw;
          }
        }
        else
        {
          LOG(WARN, "no callback handler registered, dropping message");
          // drop message (reuse allocated memory)
          in_message_ = m;
          in_message_->resize (0);
        }

        start_read ();
      }
      else
      {
        if (callback_handler_)
        {
          try
          {
            callback_handler_->handle_error (this, ec);
          }
          catch (std::exception const & ex)
          {
            LOG(ERROR, "could not pass received message to callback handler: " << ex.what());
            throw;
          }
        }
      }
    }

    void connection_t::async_send ( const message_t * msg
                                  , completion_handler_t hdl
                                  , boost::posix_time::time_duration timeout
                                  )
    {
      strand_.post (boost::bind( &self::start_send
                               , this
                               , to_send_t (msg, hdl, timeout)
                               ));
    }

    void connection_t::start_send (connection_t::to_send_t s)
    {
      bool send_in_progress = !to_send_.empty();
      to_send_.push_back (s);
      if (! send_in_progress)
        start_send();
    }

    void connection_t::start_send ()
    {
      assert (to_send_.size() > 0);

      const to_send_t & d (to_send_.front());

      DLOG( TRACE
          , "initiating write of " << d.message->header.length << " bytes of payload data: "
          << util::basic_hex_converter<128>::convert(d.message->data)
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

    void connection_t::handle_write (const boost::system::error_code & ec)
    {
      if (! ec)
      {
        DLOG(TRACE, "write completed");
        to_send_t d (to_send_.front());
        to_send_.pop_front();

        try
        {
          if (d.handler)
            d.handler (ec);
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "completion handler failed: " << ex.what());
        }

        if (d.message)
        {
          delete d.message; d.message = 0;
        }

        if (! to_send_.empty())
          start_send();
      }
      else
      {
        if (callback_handler_)
          callback_handler_->handle_error (this, ec);
      }
    }
  }
}
