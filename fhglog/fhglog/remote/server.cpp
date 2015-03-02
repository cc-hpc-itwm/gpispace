#include <fhglog/remote/server.hpp>

#include <fhglog/LogMacros.hpp>

#include <functional>

namespace fhg
{
  namespace log
  {
    namespace remote
    {
      LogServer::LogServer ( fhg::log::Logger& log
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

          if (ec)
          {
            LOG ( WARN
                , "could not set resuse address option: "
                << ec << ": " << ec.message()
                );
          }

          async_receive();
        }

      void LogServer::async_receive()
      {
        socket_.async_receive_from
          ( boost::asio::buffer (data_, sizeof (data_))
          , sender_endpoint_
          , [this]( const boost::system::error_code& error
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

              _log.log (LogEvent::from_string (msg));
            }

            async_receive();
          }
          );
      }
    }
  }
}
