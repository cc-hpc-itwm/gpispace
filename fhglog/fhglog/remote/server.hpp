// alexander.petry@itwm.fraunhofer.de

#ifndef FHGLOG_REMOTE_LOG_SERVER_HPP
#define FHGLOG_REMOTE_LOG_SERVER_HPP 1

#include <fhglog/Logger.hpp>
#include <fhglog/remote/appender.hpp>

#include <boost/asio.hpp>

namespace fhg
{
  namespace log
  {
    namespace remote
    {
      class LogServer
      {
      public:
        LogServer ( const fhg::log::Logger::ptr_t&
                  , boost::asio::io_service&
                  , unsigned short port
                  );

        void handle_receive_from ( const boost::system::error_code&
                                 , size_t bytes_recv
                                 );
      private:
        fhg::log::Logger::ptr_t _log;
        boost::asio::ip::udp::socket socket_;
        boost::asio::ip::udp::endpoint sender_endpoint_;

        enum { max_length = (2<<16) };
        char data_[max_length];

        void async_receive();
      };
    }
  }
}

#endif