#include "server.hpp"

#include <fhglog/fhglog.hpp>

#include <boost/bind.hpp>

namespace fhg
{
  namespace log
  {
    namespace remote
    {
      LogServer::LogServer ( const fhg::log::Logger::ptr_t& log
                           , boost::asio::io_service& io_service
                           , unsigned short port
                           )
        : _log (log)
        , socket_
          ( io_service
          , boost::asio::ip::udp::endpoint (boost::asio::ip::udp::v4(), port)
          )
        {
          boost::system::error_code ec;

          socket_.set_option ( boost::asio::socket_base::reuse_address (true)
                             , ec
                             );

          LOG_IF ( WARN, ec
                 , "could not set resuse address option: "
                 << ec << ": " << ec.message()
                 );

          async_receive();
        }

      void LogServer::async_receive()
      {
        socket_.async_receive_from
          ( boost::asio::buffer (data_, max_length)
          , sender_endpoint_
          , boost::bind ( &LogServer::handle_receive_from
                        , this
                        , boost::asio::placeholders::error
                        , boost::asio::placeholders::bytes_transferred
                        )
          );
      }

      void LogServer::handle_receive_from
        ( const boost::system::error_code& error
        , size_t bytes_recv
        )
      {
        if (!error && bytes_recv > 0)
        {
          std::string const msg (data_ , data_ + bytes_recv);

          if (msg == "QUIT deadbeef")
          {
            return;
          }

          LogEvent evt (LogEvent::from_string (msg));

          {
            std::ostringstream ostr;
            ostr << sender_endpoint_;
            evt.trace (ostr.str());
          }
          _log->log (evt);
        }

        async_receive();
      }
    }
  }
}