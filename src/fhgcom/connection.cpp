
#include <fhgcom/connection.hpp>

#include <fhglog/LogMacros.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/macros.hpp>

#include <boost/system/error_code.hpp>

#include <functional>

using namespace boost::asio::ip;

namespace fhg
{
  namespace com
  {
    connection_t::connection_t
      ( boost::asio::io_service & io_service
      , std::function<void (ptr_t connection, const message_t*)> handle_hello_message
      , std::function<void (ptr_t connection, const message_t*)> handle_user_data
      , std::function<void (ptr_t connection, const boost::system::error_code&)> handle_error
      )
      : strand_(io_service)
      , socket_(io_service)
      , _handle_hello_message (handle_hello_message)
      , _handle_user_data (handle_user_data)
      , _handle_error (handle_error)
      , in_message_(new message_t)
    {}

    connection_t::~connection_t ()
    {
      try
      {
        stop();
      }
      catch (std::exception const & ex)
      {
        LOG(ERROR, "exception during connection destructor: " << ex.what());
      }

      if (in_message_)
      {
        delete in_message_;
        in_message_ = nullptr;
      }
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
      boost::system::error_code ec;
      if (socket_.is_open())
      {
        socket_.shutdown (boost::asio::ip::tcp::socket::shutdown_both, ec);
      }
      socket_.close(ec);
    }

    void connection_t::start_read ()
    {
      assert (in_message_ != nullptr);

      boost::asio::async_read( socket_
                             , boost::asio::buffer (&in_message_->header, sizeof(p2p::header_t))
                             , strand_.wrap
                             ( std::bind ( &connection_t::handle_read_header
                                         , shared_from_this()
                                         , std::placeholders::_1
                                         , std::placeholders::_2
                                         )
                             ));
    }

    void connection_t::handle_read_header( const boost::system::error_code & ec
                                         , std::size_t IFNDEF_NDEBUG (bytes_transferred)
                                         )
    {
      if (! ec)
      {
        assert (bytes_transferred == sizeof(p2p::header_t));

        // WORK HERE: convert for local endianess!
        LOG_IF( WARN
              , in_message_->header.length > (1<<22)
              , "incoming message is quite large (>4MB)!"
              );
        in_message_->resize ();

        boost::asio::async_read ( socket_
                                , boost::asio::buffer (in_message_->data)
                                , strand_.wrap
                                ( std::bind ( &connection_t::handle_read_data
                                            , shared_from_this()
                                            , std::placeholders::_1
                                            , std::placeholders::_2
                                            )
                                ));
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
      bool send_in_progress = !to_send_.empty();
      to_send_.push_back (s);
      if (! send_in_progress)
        start_send();
    }

    void connection_t::start_send ()
    {
      assert (to_send_.size() > 0);

      if (to_send_.empty ())
        return;

      const to_send_t & d (to_send_.front());

      try
      {
        boost::asio::async_write( socket_
                                , d.to_buffers()
                                , strand_.wrap (std::bind( &connection_t::handle_write
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
