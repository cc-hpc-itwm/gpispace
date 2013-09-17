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
        : service_(service)
        , m_destination_location (destination_location)
      {}

      virtual void update (const boost::any& arg)
      {
        try
        {
          notify (boost::any_cast<NotificationEvent> (arg));
        }
        catch (const boost::bad_any_cast& ex)
        {
          DLOG(TRACE, "NotificationService: could not update: " << ex.what());
        }
      }
      void notify (const NotificationEvent evt)
      {
        if (destination_)
        {
          destination_->append (FHGLOG_MKEVENT_HERE (TRACE, evt.encoded()));
        }
      }

      void open()
      {
        destination_.reset ( new fhg::log::ThreadedAppender
                             ( fhg::log::Appender::ptr_t
                               ( new fhg::log::remote::RemoteAppender
                                 (service_, m_destination_location)
                               )
                             )
                           );
      }

      std::string location() const
      {
        return m_destination_location;
      }

    private:
      std::string service_;
      std::string m_destination_location;
      fhg::log::Appender::ptr_t destination_;
    };
  }
}

#endif
