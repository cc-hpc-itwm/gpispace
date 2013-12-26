// alexander.petry@itwm.fraunhofer.de

#ifndef FHGLOG_REMOTE_APPENDER_HPP
#define FHGLOG_REMOTE_APPENDER_HPP 1

#include <fhglog/Appender.hpp>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#if ! defined(FHGLOG_DEFAULT_PORT)
// ascii codes of fhgl: 102 104 103 108
#define FHGLOG_DEFAULT_PORT  2438
#endif

#if ! defined(FHGLOG_DEFAULT_HOST)
#define FHGLOG_DEFAULT_HOST  "localhost"
#endif

#if ! defined(FHGLOG_DEFAULT_LOCATION)
#define FHGLOG_DEFAULT_LOCATION "localhost:2438"
#endif

namespace fhg
{
  namespace log
  {
    namespace remote
    {
      class RemoteAppender : public Appender
      {
      public:
        explicit
        RemoteAppender (const std::string &location);
        virtual ~RemoteAppender();

        const std::string& host() const
        {
          return host_;
        }
        const unsigned short& port() const
        {
          return port_;
        }

        void append (const LogEvent&);
        void flush() {}

      private:
        std::string host_;
        unsigned short port_;

        boost::asio::ip::udp::socket *socket_;
        boost::asio::io_service io_service_;
        boost::asio::ip::udp::endpoint logserver_;
      };
    }
  }
}

#endif
