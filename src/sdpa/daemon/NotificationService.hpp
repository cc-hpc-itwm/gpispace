// alexander.petry@itwm.fraunhofer.de, bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <fhglog/remote/appender.hpp>
#include <fhglog/LogMacros.hpp>

#include <util-generic/cxx14/make_unique.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <memory>

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
        : destination_ (fhg::util::cxx14::make_unique<fhg::log::remote::RemoteAppender>
                         (destination_location, appender_io_service)
                       )
      {}

      NotificationService ( std::string const& host
                          , unsigned short port
                          , boost::asio::io_service& appender_io_service
                          )
        : destination_ (fhg::util::cxx14::make_unique<fhg::log::remote::RemoteAppender>
                         (host, std::to_string (port), appender_io_service)
                       )
      {}

      void notify (const NotificationEvent evt) const
      {
        destination_->append (FHGLOG_MKEVENT_HERE (TRACE, evt.encoded()));
      }

    private:
      std::unique_ptr<fhg::log::Appender> destination_;
    };
  }
}
