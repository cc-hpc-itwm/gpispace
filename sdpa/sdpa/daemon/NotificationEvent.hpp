/*
 * =====================================================================================
 *
 *       Filename:  NotificationEvent.hpp
 *
 *    Description:  notification event
 *
 *        Version:  1.0
 *        Created:  11/19/2009 01:27:22 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_DAEMON_NOTIFICATION_EVENT_HPP
#define SDPA_DAEMON_NOTIFICATION_EVENT_HPP 1

#include <sdpa/daemon/mpl.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>

#include <string>

namespace sdpa { namespace daemon {
  class NotificationEvent
  {
  public:
    enum state_t
    {
      STATE_IGNORE = -1
    , STATE_CREATED
    , STATE_STARTED
    , STATE_FINISHED
    , STATE_FAILED
    , STATE_CANCELLED
    , STATE_MIN = STATE_IGNORE
    , STATE_MAX = STATE_CANCELLED
    };

    NotificationEvent()
      : a_state_(STATE_IGNORE)
    {}

    NotificationEvent( const std::string &source
                     , const std::string &activity_id
                     , const std::string &activity_name
                     , const state_t &activity_state
                     , const std::string &activity_encoded = ""
                     )
      : m_component (source)
      , a_id_(activity_id)
      , a_name_(activity_name)
      , a_state_(activity_state)
      , a_activity_(activity_encoded)
    {}

    const std::string &component() const { return m_component; }
    std::string &component() { return m_component; }

    const std::string &activity_id() const   { return a_id_; }
    std::string &activity_id()               { return a_id_; }
    const std::string &activity_name() const { return a_name_; }
    std::string &activity_name()             { return a_name_; }
    const state_t &activity_state() const      { return a_state_; }
    state_t &activity_state()                  { return a_state_; }
    const std::string &activity() const { return a_activity_; }
    std::string &activity()             { return a_activity_; }
  private:
    std::string m_component;
    std::string a_id_;
    std::string a_name_;
    state_t     a_state_;
    std::string a_activity_;
  };
}}

BOOST_CLASS_TRACKING ( sdpa::daemon::NotificationEvent
                     , boost::serialization::track_never
                     )

namespace boost { namespace serialization {
  template <class Archive>
  void serialize(Archive &ar, sdpa::daemon::NotificationEvent &e, const unsigned int)
  {
    ar & e.component();
    ar & e.activity_id();
    ar & e.activity_name();
    ar & e.activity_state();
    ar & e.activity();
  }
}}

#endif
