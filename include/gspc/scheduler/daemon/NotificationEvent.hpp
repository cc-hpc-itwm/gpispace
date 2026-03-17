// Copyright (C) 2010,2013,2015-2016,2019-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/net.hpp> // recursive wrapper of transition_t fails otherwise.

#include <map>
#include <optional>
#include <list>
#include <string>

  namespace gspc::scheduler::daemon
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
      [[nodiscard]] static auto to_string (state_t) -> std::string;

      using ColorsForState = std::map<state_t, std::string>;

      NotificationEvent ( std::list<std::string> const& components
                        , std::string const& activity_id
                        , state_t const& activity_state
                        , gspc::we::type::Activity const&
                        );

      NotificationEvent (std::string encoded);
      std::string encoded() const;

      std::list<std::string> const& components() const { return _components; }
      std::string const& activity_id() const { return _activity_id; }
      std::string const& activity_name() const { return _activity_name; }
      state_t const& activity_state() const { return _activity_state; }
      [[nodiscard]] auto colors_for_state() const { return _colors_for_state; }
      std::optional<std::string> const& tooltip() const { return _tooltip; }

    private:
      std::list<std::string> _components;
      std::string _activity_id;
      std::string _activity_name;
      state_t _activity_state;
      ColorsForState _colors_for_state;
      std::optional<std::string> _tooltip;
    };

    constexpr char const* const gantt_log_category = "gantt-job-events";
  }
