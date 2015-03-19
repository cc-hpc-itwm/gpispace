// alexander.petry@itwm.fraunhofer.de

#include <fhglog/remote/appender.hpp>

#include <util-generic/split.hpp>

namespace fhg
{
  namespace log
  {
    namespace remote
    {
      using boost::asio::ip::udp;

      RemoteAppender::RemoteAppender ( const std::string& location
                                     , boost::asio::io_service& io_service
                                     )
        : RemoteAppender ( fhg::util::split_string (location, ':').first
                         , fhg::util::split_string (location, ':').second
                         , io_service
                         )
      {}

      RemoteAppender::RemoteAppender ( std::string const& host
                                     , std::string const& port
                                     , boost::asio::io_service& io_service
                                     )
        : io_service_ (io_service)
        , logserver_ (*udp::resolver (io_service_)
                     . resolve
                       ( udp::resolver::query
                           (udp::v4(), host, port, udp::resolver::query::flags())
                       )
                     )
        , socket_ (new udp::socket (io_service_, udp::v4()))
      {}

      RemoteAppender::~RemoteAppender()
      {
        socket_->close();
      }

      void RemoteAppender::append (const LogEvent& event)
      {
        boost::system::error_code ignored_error;

        socket_->send_to ( boost::asio::buffer (event.encoded())
                         , logserver_
                         , 0
                         , ignored_error
                         );
      }
    }
  }
}
