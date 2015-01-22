// alexander.petry@itwm.fraunhofer.de

#ifndef FHGLOG_REMOTE_LOG_SERVER_HPP
#define FHGLOG_REMOTE_LOG_SERVER_HPP 1

#include <fhglog/Logger.hpp>

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

      private:
        fhg::log::Logger::ptr_t _log;
        boost::asio::ip::udp::socket socket_;
        boost::asio::ip::udp::endpoint sender_endpoint_;

        char data_[2 << 16];

        void async_receive();
      };
    }
  }
}

#endif
