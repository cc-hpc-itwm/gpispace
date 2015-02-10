// alexander.petry@itwm.fraunhofer.de, bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_UTIL_NOTIFICATION_SERVICE_HPP
#define SDPA_UTIL_NOTIFICATION_SERVICE_HPP

#include <fhglog/remote/appender.hpp>
#include <fhglog/LogMacros.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

namespace sdpa
{
  namespace daemon
  {
    class NotificationService
    {
    public:
      NotificationService ( const std::string& destination_location
                          , boost::asio::io_service& appender_io_service
                          )
        : destination_ (new fhg::log::remote::RemoteAppender
                         (destination_location, appender_io_service)
                       )
      {}

      NotificationService ( std::string const& host
                          , unsigned short port
                          , boost::asio::io_service& appender_io_service
                          )
        : destination_ (new fhg::log::remote::RemoteAppender
                         (host, std::to_string (port), appender_io_service)
                       )
      {}

      void notify (const NotificationEvent evt) const
      {
        destination_->append (FHGLOG_MKEVENT_HERE (TRACE, evt.encoded()));
      }

    private:
      fhg::log::Appender::ptr_t destination_;
    };
  }
}

#endif
