// alexander.petry@itwm.fraunhofer.de

#ifndef FHGLOG_REMOTE_APPENDER_HPP
#define FHGLOG_REMOTE_APPENDER_HPP 1

#include <fhglog/Appender.hpp>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

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

        void append (const LogEvent&);
        void flush() {}

      private:
        boost::asio::io_service io_service_;
        boost::asio::ip::udp::endpoint logserver_;
        boost::asio::ip::udp::socket *socket_;
      };
    }
  }
}

#endif
