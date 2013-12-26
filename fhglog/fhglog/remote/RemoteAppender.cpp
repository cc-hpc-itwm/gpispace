// alexander.petry@itwm.fraunhofer.de

#include <fhglog/remote/RemoteAppender.hpp>

#include <fhglog/fhglog.hpp>

#include <fhg/util/split.hpp>

#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>

#include <sstream>

namespace fhg
{
  namespace log
  {
    namespace remote
    {
      RemoteAppender::RemoteAppender (const std::string& location)
        : socket_ (NULL)
      {
        std::pair<std::string, std::string> host_port
          (fhg::util::split_string(location, ":"));

        host_ = host_port.first;

       if (host_port.second.empty())
        {
          host_port.second = boost::lexical_cast<std::string>(FHGLOG_DEFAULT_PORT);
        }

        std::stringstream sstr (host_port.second);
        sstr >> port_;
        if (!sstr)
        {
          throw std::runtime_error
            ("could not parse port information: " + host_port.second);
        }

        using boost::asio::ip::udp;

        udp::resolver resolver (io_service_);
        udp::resolver::query query (udp::v4(), host_.c_str(), "0");
        logserver_ = *resolver.resolve (query);
        logserver_.port (port_);

        socket_ = new udp::socket (io_service_, udp::v4());
      }

      RemoteAppender::~RemoteAppender()
      {
        if (socket_)
        {
          socket_->close();
          delete socket_;
          socket_ = NULL;
        }
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
