#include <sdpa/daemon/NotificationEvent.hpp>

#include <we/type/net.hpp> // recursive wrapper of transition_t fails otherwise.
#include <we/type/activity.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <sstream>

namespace sdpa
{
  namespace daemon
  {
    NotificationEvent::NotificationEvent
        ( std::list<std::string> components
        , std::string activity_id
        , state_t activity_state
        , we::type::activity_t const& activity
        )
      : _components (std::move (components))
      , _activity_id (std::move (activity_id))
      , _activity_name (activity.transition().name())
      , _activity_state (std::move (activity_state))
      , _activity_transition_id (activity.transition_id())
      , _activity_submission_ts (activity.timestamp())
    {}

    NotificationEvent::NotificationEvent (std::string const& encoded)
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
    std::string NotificationEvent::encoded() const
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

    std::list<std::string> const& NotificationEvent::components() const
    {
      return _components;
    }
    std::string const& NotificationEvent::activity_id() const
    {
      return _activity_id;
    }
    std::string const& NotificationEvent::activity_name() const
    {
      return _activity_name;
    }
    NotificationEvent::state_t const& NotificationEvent::activity_state() const
    {
      return _activity_state;
    }
    boost::optional<we::transition_id_type> const&
      NotificationEvent::activity_transition_id() const
    {
      return _activity_transition_id;
    }
    boost::optional<we::type::timestamp_t> const&
      NotificationEvent::activity_submission_ts() const
    {
      return _activity_submission_ts;
    }
  }
}
