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
      using boost::asio::ip::udp;

      RemoteAppender::RemoteAppender (const std::string& location)
        : socket_ (NULL)
        , logserver_ (*udp::resolver (io_service_)
                     . resolve
                       ( udp::resolver::query
                         ( udp::v4()
                         , fhg::util::split_string (location, ":").first.c_str()
                         , "0"
                         )
                       )
                     )
      {
        logserver_.port ( boost::lexical_cast<unsigned long>
                          (fhg::util::split_string (location, ":").second)
                        );

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
