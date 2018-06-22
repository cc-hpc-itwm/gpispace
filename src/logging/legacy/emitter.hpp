#pragma once

#include <fhglog/remote/appender.hpp>

#include <boost/asio/io_service.hpp>

#include <string>

namespace fhg
{
  namespace logging
  {
    namespace legacy
    {
      using namespace ::fhg::log;

      class emitter
      {
      public:
        emitter (std::string const& hostname, unsigned short port);

        void trace (std::string const&);
        void info (std::string const&);
        void warn (std::string const&);
        void error (std::string const&);

      private:
        boost::asio::io_service _io_service;
        remote::RemoteAppender _appender;
      };
    }
  }
}
