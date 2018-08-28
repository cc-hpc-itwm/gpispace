#pragma once

#include <we/type/id.hpp>

#include <boost/optional.hpp>

#include <list>
#include <string>

namespace we
{
  namespace type
  {
    class activity_t;
    using timestamp_t = double;
  }
}

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
      };

      enum class type_t
      {
        agent,
        module_call,
        vmem_get,
        vmem_put,
      };

      NotificationEvent ( std::list<std::string> components
                        , std::string activity_id
                        , state_t activity_state
                        , type_t
                        , we::type::activity_t const& activity
                        );

      NotificationEvent (std::string const& encoded);
      std::string encoded() const;

      std::list<std::string> const& components() const;
      std::string const& activity_id() const;
      std::string const& activity_name() const;
      state_t const& activity_state() const;
      type_t const& type() const;
      boost::optional<we::transition_id_type> const&
        activity_transition_id() const;
      boost::optional<we::type::timestamp_t> const&
        activity_submission_ts() const;

    private:
      std::list<std::string> _components;
      std::string _activity_id;
      std::string _activity_name;
      state_t _activity_state;
      type_t _type;
      boost::optional<we::transition_id_type> _activity_transition_id;
      boost::optional<we::type::timestamp_t> _activity_submission_ts;
    };
  }
}
