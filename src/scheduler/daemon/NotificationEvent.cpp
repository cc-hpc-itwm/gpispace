// Copyright (C) 2010,2013,2015-2016,2019-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <functional>
#include <iomanip>
#include <gspc/scheduler/daemon/NotificationEvent.hpp>
#include <sstream>
#include <utility>
#include <gspc/we/type/Expression.hpp>
#include <gspc/we/type/property.hpp>
#include <gspc/we/type/value/show.hpp>

namespace gspc::scheduler::daemon
{
  namespace
  {
    template<typename PrintValue>
      [[nodiscard]] auto monitor_property
        ( gspc::we::type::Activity const& activity
        , std::list<std::string> keys
        , PrintValue&& print_value
        ) -> std::optional<std::string>
    {
      auto const& transition {activity.transition()};

      auto path {std::list<std::string> {"gspc", "monitor"}};
      path.splice (std::end (path), keys);

      if (auto const expression {transition.prop().get (path)})
      {
        auto context {gspc::we::expr::eval::context{}};

        for (auto const& input : activity.input())
        {
          context.bind_ref
            ( transition.ports_input().at (input._port_id).name()
            , input._token
            );
        }

        return std::invoke
          ( std::forward<PrintValue> (print_value)
          , gspc::we::type::Expression
            { ::boost::get<std::string> (expression->get())
            }.ast().eval_all (context)
          );
      }

      return {};
    }

    [[nodiscard]] auto color_property
      ( gspc::we::type::Activity const& activity
      , std::string const& state_name
      ) -> std::optional<std::string>
    {
      return monitor_property
        ( activity
        , {"color", state_name}
        , [] (auto color)
          {
            auto oss {std::ostringstream{}};
            oss << "#" << std::hex
                << std::setfill ('0')
                << std::setw (6)
                << (::boost::get<int> (color) % 0xFFFFFF)
              ;
            return oss.str();
          }
        );
    }

    [[nodiscard]] auto make_colors (gspc::we::type::Activity const& activity)
      -> NotificationEvent::ColorsForState
    {
      auto colors {NotificationEvent::ColorsForState{}};

      auto const add_color_for_state
        { [&] (auto state)
          {
            auto const name {NotificationEvent::to_string (state)};

            if (auto const color {color_property (activity, name)})
            {
              colors.emplace (state, *color);
            }
          }
        };

      add_color_for_state (NotificationEvent::STATE_STARTED);
      add_color_for_state (NotificationEvent::STATE_FINISHED);
      add_color_for_state (NotificationEvent::STATE_FAILED);
      add_color_for_state (NotificationEvent::STATE_CANCELED);

      return colors;
    }
  }

  auto NotificationEvent::to_string (state_t state) -> std::string
  {
    switch (state)
    {
      break; case STATE_STARTED: return "started";
      break; case STATE_FINISHED: return "finished";
      break; case STATE_FAILED: return "failed";
      break; case STATE_CANCELED: return "canceled";
    }

    throw std::logic_error {"unknown state"};
  }

  NotificationEvent::NotificationEvent
    ( std::list<std::string> const& components
    , std::string const& activity_id
    , state_t const& activity_state
    , gspc::we::type::Activity const& activity
    )
      : _components (components)
      , _activity_id (activity_id)
      , _activity_name (activity.name())
      , _activity_state (activity_state)
      , _colors_for_state (make_colors (activity))
      , _tooltip
        { monitor_property
          ( activity
          , {"tooltip"}
          , [] (auto tooltip)
            {
              auto oss {std::ostringstream{}};
              oss << gspc::pnet::type::value::show (tooltip);
              return oss.str();
            }
          )
        }
  {}

  NotificationEvent::NotificationEvent (std::string encoded)
  {
    std::istringstream stream (encoded);
    ::boost::archive::text_iarchive archive (stream);
    archive & _components;
    archive & _activity_id;
    archive & _activity_name;
    archive & _activity_state;
    archive & _colors_for_state;
    archive & _tooltip;
  }
  auto NotificationEvent::encoded() const -> std::string
  {
    std::ostringstream stream;
    ::boost::archive::text_oarchive archive (stream);
    archive & _components;
    archive & _activity_id;
    archive & _activity_name;
    archive & _activity_state;
    archive & _colors_for_state;
    archive & _tooltip;
    return stream.str();
  }
}
