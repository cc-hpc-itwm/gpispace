// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <we/type/net.hpp> // recursive wrapper of transition_t fails otherwise.
#include <we/type/activity.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

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

      NotificationEvent ( const std::list<std::string>& components
                        , const std::string& activity_id
                        , const state_t& activity_state
                        , const we::type::activity_t& activity
                        )
        : _components (components)
        , _activity_id (activity_id)
        , _activity_name (activity.name())
        , _activity_state (activity_state)
      {}

      NotificationEvent (const std::string encoded)
      {
        std::istringstream stream (encoded);
        boost::archive::text_iarchive archive (stream);
        archive & _components;
        archive & _activity_id;
        archive & _activity_name;
        archive & _activity_state;
      }
      std::string encoded() const
      {
        std::ostringstream stream;
        boost::archive::text_oarchive archive (stream);
        archive & _components;
        archive & _activity_id;
        archive & _activity_name;
        archive & _activity_state;
        return stream.str();
      }

      const std::list<std::string>& components() const { return _components; }
      const std::string &activity_id() const { return _activity_id; }
      const std::string &activity_name() const { return _activity_name; }
      const state_t &activity_state() const { return _activity_state; }

    private:
      std::list<std::string> _components;
      std::string _activity_id;
      std::string _activity_name;
      state_t _activity_state;
    };

    constexpr char const* const gantt_log_category = "gantt-job-events";
  }
}
