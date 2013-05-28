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
  template <class Service>
  class notify_helper
  {
  public:
    typedef typename Service::event_t event_t;
    typedef typename event_t::state_t state_t;

    explicit notify_helper(Service* s, const std::string& activity_encoded)
      : service_(s)
      , event_()
    {
      event_.activity() = activity_encoded;
    }
    notify_helper& component (const std::string& component)
    {
      event_.component() = component;
      return *this;
    }

    notify_helper &activity_id(const std::string &id)     { event_.activity_id() = id; return *this; }
    notify_helper &activity_name(const std::string &name) { event_.activity_name() = name; return *this; }
    notify_helper &activity_state(state_t state)          { event_.activity_state() = state; return *this; }

    notify_helper &activity_created()
    {
      event_.activity_state() = NotificationEvent::STATE_CREATED; return *this;
    }
    notify_helper &activity_finished()
    {
      event_.activity_state() = NotificationEvent::STATE_FINISHED; return *this;
    }
    notify_helper &activity_failed()
    {
      event_.activity_state() = NotificationEvent::STATE_FAILED; return *this;
    }
    notify_helper &activity_cancelled()
    {
      event_.activity_state() = NotificationEvent::STATE_CANCELLED; return *this;
    }

    ~notify_helper() { try { service_->notify(event_); } catch(...) {} }
  private:
    Service* service_;
    event_t event_;
  };


  template <class EventType>
  class BasicNotificationService : public Observer
  {
  public:
    typedef EventType event_t;

    friend class notify_helper<BasicNotificationService>;

    BasicNotificationService(const std::string &service, const std::string &destination_location)
      : service_(service)
      , m_destination_location (destination_location)
    {}

    notify_helper<BasicNotificationService> operator()() const
    {
      return notify_helper<BasicNotificationService>(*this);
    }

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

    ~BasicNotificationService() {}
  private:
    void notify(const event_t &evt)
    {
      if (destination_)
        destination_->append(FHGLOG_MKEVENT_HERE(TRACE, encode(evt)));
    }

    std::string encode(const event_t &evt) const
    {
      std::ostringstream sstr;
      boost::archive::text_oarchive ar(sstr);
      ar << evt;
      return sstr.str();
    }

    std::string service_;
    std::string m_destination_location;
    fhg::log::Appender::ptr_t destination_;
  };

  typedef BasicNotificationService<NotificationEvent> NotificationService;
}}

#endif
