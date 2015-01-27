// alexander.petry@itwm.fraunhofer.de

#ifndef FHGLOG_REMOTE_APPENDER_HPP
#define FHGLOG_REMOTE_APPENDER_HPP 1

#include <fhglog/Appender.hpp>

#include <boost/asio.hpp>

#include <memory>

namespace fhg
{
  namespace log
  {
    namespace remote
    {
      class RemoteAppender : public Appender
      {
      public:
        RemoteAppender (const std::string &location, boost::asio::io_service&);
        virtual ~RemoteAppender();

        virtual void append (const LogEvent&) override;
        virtual void flush() override {}

      private:
        boost::asio::io_service& io_service_;
        boost::asio::ip::udp::endpoint logserver_;
        std::unique_ptr<boost::asio::ip::udp::socket> socket_;
      };
    }
  }
}

#endif
