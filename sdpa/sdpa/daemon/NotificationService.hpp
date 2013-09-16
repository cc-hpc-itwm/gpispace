/*
 * =====================================================================================
 *
 *       Filename:  NotificationService.hpp
 *
 *    Description:  implements a notication service
 *
 *        Version:  1.0
 *        Created:  11/18/2009 11:48:19 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_UTIL_NOTIFICATION_SERVICE_HPP
#define SDPA_UTIL_NOTIFICATION_SERVICE_HPP 1

#include <fhglog/ThreadedAppender.hpp>
#include <fhglog/remote/RemoteAppender.hpp>
#include <sdpa/memory.hpp>

#include <sdpa/daemon/Observer.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace sdpa { namespace daemon {
  template <class EventType>
  class BasicNotificationService : public Observer
  {
  public:
    typedef EventType event_t;

    BasicNotificationService(const std::string &service, const std::string &destination_location)
      : service_(service)
      , m_destination_location (destination_location)
    {}

    void update(const boost::any &arg)
    {
      try
      {
        notify(boost::any_cast<event_t>(arg));
      }
      catch (const boost::bad_any_cast &ex)
      {
        DLOG(TRACE, "NotificationService: could not update: " << ex.what());
      }
    }

    void open ()
    {
      destination_.reset (new fhg::log::ThreadedAppender
                         (fhg::log::Appender::ptr_t
                         (new fhg::log::remote::RemoteAppender( service_
                                                              , m_destination_location
                                                              )
                         )));
    }

    std::string location() { return m_destination_location; }

    ~BasicNotificationService() {}

    void notify(const event_t &evt)
    {
      if (destination_)
        destination_->append(FHGLOG_MKEVENT_HERE(TRACE, encode(evt)));
    }

  private:
    std::string encode(const event_t &evt) const
    {
      return evt.encoded();
    }

    std::string service_;
    std::string m_destination_location;
    fhg::log::Appender::ptr_t destination_;
  };

  typedef BasicNotificationService<NotificationEvent> NotificationService;
}}

#endif
