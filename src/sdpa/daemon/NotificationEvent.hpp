#pragma once

#include <we/type/net.hpp> // recursive wrapper of transition_t fails otherwise.
#include <we/type/activity.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/optional.hpp>

#include <sstream>
#include <string>

namespace sdpa
{
  namespace daemon
  {
    class NotificationEvent
    {
    public:
      enum state_t
      {
        STATE_STARTED
      , STATE_FINISHED
      , STATE_FAILED
      , STATE_CANCELED
      , STATE_VMEM_PUT_FINISHED
      , STATE_VMEM_GET_FINISHED
      , STATE_MAX = STATE_CANCELED
      };

      NotificationEvent ( const std::list<std::string>& components
                        , const std::string& activity_id
                        , const state_t& activity_state
                        , const we::type::activity_t& activity
                        )
        : _components (components)
        , _activity_id (activity_id)
        , _activity_name (activity.transition().name())
        , _activity_state (activity_state)
        , _activity_transition_id (activity.transition_id())
        , _activity_submission_ts(activity.timestamp())
      {}

      NotificationEvent (const std::string encoded)
      {
        std::istringstream stream (encoded);
        boost::archive::text_iarchive archive (stream);
        archive & _components;
        archive & _activity_id;
        archive & _activity_name;
        archive & _activity_state;
        archive & _activity_transition_id;
        archive & _activity_submission_ts;
      }

      std::string encoded() const
      {
        std::ostringstream stream;
        boost::archive::text_oarchive archive (stream);
        archive & _components;
        archive & _activity_id;
        archive & _activity_name;
        archive & _activity_state;
        archive & _activity_transition_id;
        archive & _activity_submission_ts;
        return stream.str();
      }

      const std::list<std::string>& components() const { return _components; }
      const std::string &activity_id() const { return _activity_id; }
      const std::string &activity_name() const { return _activity_name; }
      const state_t &activity_state() const { return _activity_state; }
      const boost::optional<we::transition_id_type> &activity_transition_id() const { return _activity_transition_id; }
      const boost::optional<we::type::timestamp_t> &activity_submission_ts() const { return _activity_submission_ts; }

    private:
      std::list<std::string> _components;
      std::string _activity_id;
      std::string _activity_name;
      state_t _activity_state;
      boost::optional<we::transition_id_type> _activity_transition_id;
      boost::optional<we::type::timestamp_t> _activity_submission_ts;
    };
  }
}
