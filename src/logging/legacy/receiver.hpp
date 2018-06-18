#pragma once

#include <logging/legacy/event.hpp>

#include <fhglog/Logger.hpp>
#include <fhglog/remote/server.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/thread/scoped_thread.hpp>

namespace fhg
{
  namespace logging
  {
    namespace legacy
    {
      using namespace ::fhg::log;

      class receiver
      {
      public:
        receiver (unsigned short port);
        virtual ~receiver();

      protected:
        virtual void on_legacy (event const&) = 0;

      private:
        boost::asio::io_service _io_service;
        Logger _logger;
        remote::LogServer _server;
        boost::strict_scoped_thread<> _thread;
      };
    }
  }
}
