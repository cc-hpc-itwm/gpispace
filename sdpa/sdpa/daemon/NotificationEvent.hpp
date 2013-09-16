// alexander.petry@itwm.fraunhofer.de, bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_DAEMON_NOTIFICATION_EVENT_HPP
#define SDPA_DAEMON_NOTIFICATION_EVENT_HPP

#include <boost/serialization/utility.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/type/net.hpp> // recursive wrapper of transition_t fails otherwise.
#include <we/mgmt/type/activity.hpp>

#include <sstream>
#include <string>

namespace sdpa
{
  namespace daemon
  {
    namespace
    {
      boost::optional<std::string> nice_name (const std::string& act)
      try
      {
        const we::mgmt::type::activity_t activity (act);

        const we::type::module_call_t mod_call
          (boost::get<we::type::module_call_t> (activity.transition().data()));

        return mod_call.module() + ":" + mod_call.function();
      }
      catch (boost::bad_get const &)
      {
        return boost::none;
      }
    }

    class NotificationEvent
    {
    public:
      enum state_t
      {
        STATE_CREATED
      , STATE_STARTED
      , STATE_FINISHED
      , STATE_FAILED
      , STATE_CANCELLED
      , STATE_MIN = STATE_CREATED
      , STATE_MAX = STATE_CANCELLED
      };

      NotificationEvent
        ( const std::string &source
        , const std::string &activity_id
        , const std::string &activity_name
        , const state_t &activity_state
        , const boost::optional<std::string>& activity_encoded = boost::none
        )
          : m_component (source)
          , a_id_(activity_id)
          , a_name_ ( activity_encoded
                    ? nice_name (*activity_encoded).get_value_or (activity_name)
                    : activity_name
                    )
          , a_state_(activity_state)
      {}

      NotificationEvent (const std::string encoded)
      {
        std::istringstream stream (encoded);
        boost::archive::text_iarchive archive (stream);
        archive & m_component;
        archive & a_id_;
        archive & a_name_;
        archive & a_state_;
      }
      std::string encoded() const
      {
        std::ostringstream stream;
        boost::archive::text_oarchive archive (stream);
        archive & m_component;
        archive & a_id_;
        archive & a_name_;
        archive & a_state_;
        return stream.str();
      }

      const std::string &component() const { return m_component; }
      std::string &component() { return m_component; }

      const std::string &activity_id() const   { return a_id_; }
      std::string &activity_id()               { return a_id_; }
      const std::string &activity_name() const { return a_name_; }
      std::string &activity_name()             { return a_name_; }
      const state_t &activity_state() const      { return a_state_; }
      state_t &activity_state()                  { return a_state_; }

    private:
      std::string m_component;
      std::string a_id_;
      std::string a_name_;
      state_t     a_state_;
    };
  }
}

#endif
