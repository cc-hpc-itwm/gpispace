// alexander.petry@itwm.fraunhofer.de

#pragma once

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
        LogServer ( fhg::log::Logger&
                  , boost::asio::io_service&
                  , unsigned short port
                  );

      private:
        fhg::log::Logger& _log;
        boost::asio::ip::udp::socket socket_;
        boost::asio::ip::udp::endpoint sender_endpoint_;

        char data_[2 << 16];

        void async_receive();
      };
    }
  }
}
