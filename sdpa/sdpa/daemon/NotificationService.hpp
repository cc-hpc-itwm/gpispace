// alexander.petry@itwm.fraunhofer.de, bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_UTIL_NOTIFICATION_SERVICE_HPP
#define SDPA_UTIL_NOTIFICATION_SERVICE_HPP

#include <fhglog/ThreadedAppender.hpp>
#include <fhglog/remote/RemoteAppender.hpp>

#include <sdpa/daemon/Observer.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

namespace sdpa
{
  namespace daemon
  {
    class NotificationService : public Observer
    {
    public:
      NotificationService ( const std::string& service
                          , const std::string& destination_location
                          )
        : destination_ ( new fhg::log::ThreadedAppender
                         ( fhg::log::Appender::ptr_t
                           ( new fhg::log::remote::RemoteAppender
                             (service, destination_location)
                           )
                         )
                       )
      {}

      virtual void update (const boost::any& arg)
      {
        notify (boost::any_cast<NotificationEvent> (arg));
      }
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
