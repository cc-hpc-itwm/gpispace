// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/Activity.hpp>
#include <we/type/net.hpp> // recursive wrapper of transition_t fails otherwise.

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

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
      , STATE_MAX = STATE_CANCELED
      };

      NotificationEvent ( std::list<std::string> const& components
                        , std::string const& activity_id
                        , state_t const& activity_state
                        , we::type::Activity const& activity
                        )
        : _components (components)
        , _activity_id (activity_id)
        , _activity_name (activity.name())
        , _activity_state (activity_state)
      {}

      NotificationEvent (std::string encoded)
      {
        std::istringstream stream (encoded);
        ::boost::archive::text_iarchive archive (stream);
        archive & _components;
        archive & _activity_id;
        archive & _activity_name;
        archive & _activity_state;
      }
      std::string encoded() const
      {
        std::ostringstream stream;
        ::boost::archive::text_oarchive archive (stream);
        archive & _components;
        archive & _activity_id;
        archive & _activity_name;
        archive & _activity_state;
        return stream.str();
      }

      std::list<std::string> const& components() const { return _components; }
      std::string const& activity_id() const { return _activity_id; }
      std::string const& activity_name() const { return _activity_name; }
      state_t const& activity_state() const { return _activity_state; }

    private:
      std::list<std::string> _components;
      std::string _activity_id;
      std::string _activity_name;
      state_t _activity_state;
    };

    constexpr char const* const gantt_log_category = "gantt-job-events";
  }
}
