#include <fhglog/minimal.hpp>

#include "connection.hpp"

using namespace boost::asio::ip;

namespace fhg
{
  namespace com
  {
    connection_t::connection_t( boost::asio::io_service & io_service
                              , message_handler_t & message_handler
                              )
      : strand_(io_service)
      , socket_(io_service)
      , deadline_(io_service)
      , message_handler_(message_handler)
    {}

    connection_t::~connection_t ()
    {
      DLOG(TRACE, "connection destroyed");
    }

    tcp::socket & connection_t::socket()
    {
      return socket_;
    }

    void connection_t::start()
    {
      start_read();
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
            return;
          }
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
                                              , shared_from_this()
                                              , boost::asio::placeholders::error
                                              , boost::asio::placeholders::bytes_transferred
                                              )
                                ));
      }
      else
      {
        message_handler_.handle_error (ec);
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
        message_handler_.handle_recv ( m );

        start_read ();
      }
      else
      {
        if (in_data_.msg)
        {
          delete in_data_.msg;
          in_data_.msg = 0;
        }
        message_handler_.handle_error (ec);
      }
    }

    void connection_t::start_read ()
    {
      boost::asio::async_read( socket_
                             , boost::asio::buffer(in_data_.header, header_length)
                             , strand_.wrap
                             ( boost::bind ( &self::handle_read_header
                                           , shared_from_this()
                                           , boost::asio::placeholders::error
                                           , boost::asio::placeholders::bytes_transferred
                                           )
                             ));
    }
  }
}
