// Copyright (C) 2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/scheduler/daemon/NotificationEvent.hpp>

#include <gspc/logging/stream_emitter.hpp>
#include <gspc/logging/stream_receiver.hpp>
#include <test/logging/message.hpp>

#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/Transition.hpp>
#include <gspc/we/type/property.hpp>
#include <gspc/we/type/value.hpp>
#include <gspc/we/type/value/poke.hpp>

#include <gspc/testing/random.hpp>

#include <gspc/rpc/remote_function.hpp>
#include <gspc/rpc/remote_tcp_endpoint.hpp>

#include <gspc/testing/printer/list.hpp>
#include <gspc/testing/printer/future.hpp>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <gspc/we/type/value/show.formatter.hpp>
#include <condition_variable>
#include <fmt/format.h>
#include <functional>
#include <future>
#include <iterator>
#include <mutex>
#include <optional>
#include <string>

namespace gspc::scheduler::daemon
{
  namespace
  {
    [[nodiscard]] auto random_identifier() -> std::string
    {
      using RandomString = gspc::testing::random<std::string>;
      static auto random_string {RandomString{}};
      return std::invoke (random_string, RandomString::identifier{});
    }

    [[nodiscard]] auto random_activity_id() -> std::string
    {
      return std::invoke (random_identifier);
    }

    [[nodiscard]] auto random_transition_name() -> std::string
    {
      return std::invoke (random_identifier);
    }

    [[nodiscard]] auto random_port_name() -> std::string
    {
      return std::invoke (random_identifier);
    }

    [[nodiscard]] auto random_input_value() -> gspc::pnet::type::value::value_type
    {
      return gspc::testing::random<long>{}();
    }

    [[nodiscard]] auto random_components() -> std::list<std::string>
    {
      auto const random_worker_name {random_identifier};

      auto components {std::list<std::string>{}};

      std::generate_n
        ( std::inserter (components, std::end (components))
        , gspc::testing::random<std::size_t>{} (10, 0)
        , random_worker_name
        );

      return components;
    }

    [[nodiscard]] auto var (std::string name) -> std::string
    {
      return fmt::format ("${{{}}}", name);
    }
    [[nodiscard]] auto var
      ( std::string name
      , std::string field
      ) -> std::string
    {
      return fmt::format ("${{{}.{}}}", name, field);
    }

    // An Activity with custom GANTT monitor properties. This
    // simulates what happens when a Petri net has monitor
    // annotations.
    //
    [[nodiscard]] auto activity_with_monitor_annotations
      ( std::optional<std::string> color_started_expr
      , std::optional<std::string> color_finished_expr
      , std::optional<std::string> color_failed_expr
      , std::optional<std::string> color_canceled_expr
      , std::optional<std::string> tooltip_expr
      , std::string input_port_name
      , gspc::pnet::type::value::value_type input_value
      ) -> gspc::we::type::Activity
    {
      auto const name {std::invoke (random_transition_name)};
      auto properties {gspc::we::type::property::type{}};

      if (color_started_expr)
      {
        properties.set
          ({"gspc", "monitor", "color", "started"}, *color_started_expr);
      }
      if (color_finished_expr)
      {
        properties.set
          ({"gspc", "monitor", "color", "finished"}, *color_finished_expr);
      }
      if (color_failed_expr)
      {
        properties.set
          ({"gspc", "monitor", "color", "failed"}, *color_failed_expr);
      }
      if (color_canceled_expr)
      {
        properties.set
          ({"gspc", "monitor", "color", "canceled"}, *color_canceled_expr);
      }
      if (tooltip_expr)
      {
        properties.set
          ({"gspc", "monitor", "tooltip"}, *tooltip_expr);
      }

      auto transition
        { gspc::we::type::Transition
          { name
          , gspc::we::type::Expression{}
          , std::nullopt
          , properties
          , gspc::we::priority_type{}
          , std::optional<gspc::we::type::eureka_id_type>{}
          , std::list<gspc::we::type::Preference>{}
          , gspc::we::type::track_shared{}
          }
        };
      transition.add_port
        ( gspc::we::type::Port
          { input_port_name
          , gspc::we::type::port::direction::In{}
          , std::string {"long"}
          , gspc::we::type::property::type{}
          }
        );

      auto activity {gspc::we::type::Activity {transition}};
      activity.add_input (input_port_name, input_value);

      return activity;
    }

    // An Activity without any monitor annotations.
    //
    [[nodiscard]] auto activity_without_annotations() -> gspc::we::type::Activity
    {
      return gspc::we::type::Activity
        { gspc::we::type::Transition
          { std::invoke (random_transition_name)
          , gspc::we::type::Expression{}
          , std::nullopt
          , gspc::we::type::property::type{}
          , gspc::we::priority_type{}
          , std::optional<gspc::we::type::eureka_id_type>{}
          , std::list<gspc::we::type::Preference>{}
          , gspc::we::type::track_shared{}
          }
        };
    }

    auto const all_activity_states
      { std::vector<NotificationEvent::state_t>
        { NotificationEvent::STATE_STARTED
        , NotificationEvent::STATE_FINISHED
        , NotificationEvent::STATE_FAILED
        , NotificationEvent::STATE_CANCELED
        }
      };
  }

  BOOST_DATA_TEST_CASE
    ( notification_event_without_annotations_has_no_custom_colors
    , all_activity_states
    , state
    )
  {
    auto const event
      { NotificationEvent
        { std::invoke (random_components)
        , std::invoke (random_activity_id)
        , state
        , std::invoke (activity_without_annotations)
        }
      };

    BOOST_REQUIRE (event.colors_for_state().empty());
    BOOST_REQUIRE (!event.tooltip().has_value());
  }

  BOOST_AUTO_TEST_CASE (notification_event_extracts_static_color_annotation)
  {
    // Static color expression: constant hex value.
    //
    auto const event
      { NotificationEvent
        { std::invoke (random_components)
        , std::invoke (random_activity_id)
        , NotificationEvent::STATE_STARTED
        , std::invoke
          ( activity_with_monitor_annotations
          , "0xFF0000"  // started: red
          , "0x00FF00"  // finished: green
          , std::nullopt
          , std::nullopt
          , std::nullopt
          , std::invoke (random_port_name)
          , std::invoke (random_input_value)
          )
        }
      };

    auto const& colors {event.colors_for_state()};

    // Has colors for started and finished.
    BOOST_REQUIRE_EQUAL (colors.count (NotificationEvent::STATE_STARTED), 1);
    BOOST_REQUIRE_EQUAL (colors.count (NotificationEvent::STATE_FINISHED), 1);

    // Color values are formatted as #RRGGBB.
    BOOST_REQUIRE_EQUAL
      (colors.at (NotificationEvent::STATE_STARTED), "#ff0000");
    BOOST_REQUIRE_EQUAL
      (colors.at (NotificationEvent::STATE_FINISHED), "#00ff00");

    // Other states don't have custom colors.
    BOOST_REQUIRE_EQUAL (colors.count (NotificationEvent::STATE_FAILED), 0);
    BOOST_REQUIRE_EQUAL (colors.count (NotificationEvent::STATE_CANCELED), 0);
  }

  BOOST_AUTO_TEST_CASE
    ( notification_event_extracts_dynamic_color_based_on_input
    )
  {
    // Dynamic color expression that uses the input value Color
    // formula: 0x010000 * value => value becomes the red component
    //
    auto const value {gspc::testing::random<long>{} (0xFFL, 0L)};
    auto const port_name {std::invoke (random_port_name)};
    auto const event
      { NotificationEvent
        { std::invoke (random_components)
        , std::invoke (random_activity_id)
        , NotificationEvent::STATE_STARTED
        , std::invoke
          ( activity_with_monitor_annotations
          // started: depends on input
          , "int (0x010000L * ${" + port_name + "})"
          , std::nullopt
          , std::nullopt
          , std::nullopt
          , std::nullopt
          , port_name
          , value
          )
        }
      };

    auto const& colors {event.colors_for_state()};

    BOOST_REQUIRE_EQUAL
      (colors.count (NotificationEvent::STATE_STARTED), 1);

    auto const expected {fmt::format ("#{:06x}", value * 0x010000L)};
    BOOST_REQUIRE_EQUAL
      (colors.at (NotificationEvent::STATE_STARTED), expected);
  }

  BOOST_DATA_TEST_CASE
    ( notification_event_extracts_static_tooltip_annotation
    , all_activity_states
    , state
    )
  {
    auto const tooltip_expression
      { fmt::format ("\"{}\"", std::invoke (random_identifier))
      };

    auto const event
      { NotificationEvent
        { std::invoke (random_components)
        , std::invoke (random_activity_id)
        , state
        , std::invoke
          ( activity_with_monitor_annotations
          , std::nullopt
          , std::nullopt
          , std::nullopt
          , std::nullopt
          , tooltip_expression
          , std::invoke (random_port_name)
          , std::invoke (random_input_value)
          )
        }
      };

    BOOST_REQUIRE (event.tooltip().has_value());
    BOOST_REQUIRE_EQUAL (event.tooltip().value(), tooltip_expression);
  }

  BOOST_DATA_TEST_CASE
    ( notification_event_extracts_dynamic_tooltip_based_on_input
    , all_activity_states
    , state
    )
  {
    // Dynamic tooltip expression that uses the input value Uses
    // struct field assignment syntax to build a tooltip with the
    // value.
    //
    auto const port_name {std::invoke (random_port_name)};
    auto const input_value {std::invoke (random_input_value)};
    auto const tooltip_variable_name {std::invoke (random_identifier)};
    auto const tooltip_value_field_name {std::invoke (random_identifier)};

    auto const event
      { NotificationEvent
        { std::invoke (random_components)
        , std::invoke (random_activity_id)
        , state
        , std::invoke
          ( activity_with_monitor_annotations
          , std::nullopt
          , std::nullopt
          , std::nullopt
          , std::nullopt
          , fmt::format
            ( "{0} := {1}; {2}"
            , var (tooltip_variable_name, tooltip_value_field_name)
            , var (port_name)
            , var (tooltip_variable_name)
            )
          , port_name
          , input_value
          )
        }
      };

    // The tooltip is a struct with a value field containing the input
    // value.
    auto const expected_tooltip
      { std::invoke
        ( [&]
          {
            auto tooltip {gspc::pnet::type::value::value_type{}};
            gspc::pnet::type::value::poke
              ( tooltip_value_field_name
              , tooltip
              , input_value
              );
            return fmt::format ("{}", gspc::pnet::type::value::show (tooltip));
          }
        )
      };
    BOOST_REQUIRE (event.tooltip().has_value());
    BOOST_REQUIRE_EQUAL (event.tooltip().value(), expected_tooltip);
  }

  BOOST_AUTO_TEST_CASE (notification_event_extracts_all_annotations_combined)
  {
    // All annotations together: static and dynamic colors, and
    // dynamic tooltip
    //
    auto const port_name {std::invoke (random_port_name)};
    // Use bounded value to keep color calculation within valid range
    auto const input_value
      { gspc::testing::random<long>{} (0xFFL, 0L)
      };
    auto const activity
      { activity_with_monitor_annotations
        // started: red-ish (static)
        ( "0xAA0000"
        // finished: green + offset (dynamic)
        , "int (0x00AA00L + ${" + port_name + "} * 0x10L)"
        , "0x0000FF"  // failed: blue (static)
        , "0x888888"  // canceled: gray (static)
        // tooltip with value
        , "${tooltip.id} := ${" + port_name + "}; ${tooltip}"
        , port_name
        , input_value
        )
      };
    auto const components {std::invoke (random_components)};

    auto const event
      { NotificationEvent
        { components
        , std::invoke (random_activity_id)
        , NotificationEvent::STATE_STARTED
        , activity
        }
      };

    // Check all colors are present
    auto const& colors {event.colors_for_state()};
    BOOST_REQUIRE_EQUAL (colors.size(), 4);

    BOOST_REQUIRE_EQUAL
      ( colors.at (NotificationEvent::STATE_STARTED)
      , "#aa0000"
      );
    // 0x00AA00 + input_value * 0x10
    auto const expected_finished
      { fmt::format ( "#{:06x}"
                    , (0x00AA00L + input_value * 0x10L) % 0xFFFFFF
                    )
      };
    BOOST_REQUIRE_EQUAL
      ( colors.at (NotificationEvent::STATE_FINISHED)
      , expected_finished
      );
    BOOST_REQUIRE_EQUAL
      ( colors.at (NotificationEvent::STATE_FAILED)
      , "#0000ff"
      );
    BOOST_REQUIRE_EQUAL
      ( colors.at (NotificationEvent::STATE_CANCELED)
      , "#888888"
      );

    // Check tooltip contains the value
    auto const expected_tooltip
      { std::invoke
        ( [&]
          {
            auto tooltip {gspc::pnet::type::value::value_type{}};
            gspc::pnet::type::value::poke ("id", tooltip, input_value);
            return fmt::format ("{}", gspc::pnet::type::value::show (tooltip));
          }
        )
      };
    BOOST_REQUIRE (event.tooltip().has_value());
    BOOST_REQUIRE_EQUAL (event.tooltip().value(), expected_tooltip);

    // Check activity name is preserved
    BOOST_REQUIRE_EQUAL (event.activity_name(), activity.name());

    // Check components are preserved
    BOOST_REQUIRE_EQUAL (event.components(), components);
  }

  BOOST_DATA_TEST_CASE
    ( notification_event_serialization_preserves_annotations
    , all_activity_states
    , state
    )
  {
    auto const original
      { NotificationEvent
        { std::invoke (random_components)
        , std::invoke (random_activity_id)
        , state
        , std::invoke
          ( activity_with_monitor_annotations
          , "0x112233"
          , "0x445566"
          , "0x778899"
          , "0xAABBCC"
          , fmt::format ("\"{}\"", std::invoke (random_identifier))
          , std::invoke (random_port_name)
          , std::invoke (random_input_value)
          )
        }
      };

    // Serialize and deserialize
    auto const decoded {NotificationEvent {original.encoded()}};

    // Verify all fields are preserved
    BOOST_REQUIRE_EQUAL (decoded.activity_id(), original.activity_id());
    BOOST_REQUIRE_EQUAL (decoded.activity_name(), original.activity_name());
    BOOST_REQUIRE_EQUAL (decoded.activity_state(), original.activity_state());
    BOOST_REQUIRE_EQUAL
      ( decoded.colors_for_state().size()
      , original.colors_for_state().size()
      );

    for (auto const& [state, color] : original.colors_for_state())
    {
      BOOST_REQUIRE_EQUAL (decoded.colors_for_state().count (state), 1);
      BOOST_REQUIRE_EQUAL (decoded.colors_for_state().at (state), color);
    }

    BOOST_REQUIRE (decoded.tooltip().has_value());
    BOOST_REQUIRE_EQUAL (*decoded.tooltip(), *original.tooltip());
  }

  BOOST_DATA_TEST_CASE
    ( notification_event_color_modulo_operation
    , all_activity_states
    , state
    )
  {
    // Test that colors are taken modulo 0xFFFFFF
    auto const event
      { NotificationEvent
        { std::invoke (random_components)
        , std::invoke (random_activity_id)
        , state
        , std::invoke
          ( activity_with_monitor_annotations
          // 0x1FFFFFF % 0xFFFFFF = 0x1000000 % 0xFFFFFF = 1
          , "0x1FFFFFF"
          , "0x1FFFFFF"
          , "0x1FFFFFF"
          , "0x1FFFFFF"
          , std::nullopt
          , std::invoke (random_port_name)
          , std::invoke (random_input_value)
          )
        }
      };

    auto const& colors {event.colors_for_state()};
    // 0x1FFFFFF % 0xFFFFFF = 0x1000000 % 0xFFFFFF = 1
    BOOST_REQUIRE_EQUAL (colors.at (state), "#000001");
  }

  BOOST_DATA_TEST_CASE
    ( custom_log_sink_receives_gantt_events_with_annotations
    , all_activity_states
    , state
    )
  {
    auto const event
      { NotificationEvent
        { std::invoke (random_components)
        , std::invoke (random_activity_id)
        , state
        , std::invoke
          ( activity_with_monitor_annotations
          , "0xFF0000"
          , "0x00FF00"
          , std::nullopt
          , std::nullopt
          , "\"Custom logging test\""
          , std::invoke (random_port_name)
          , std::invoke (random_input_value)
          )
        }
      };

    auto emitter {gspc::logging::stream_emitter{}};

    // A custom log sink that receives the message.
    auto received_promise {std::promise<gspc::logging::message>{}};
    auto received_future {received_promise.get_future()};

    auto receiver
      { gspc::logging::stream_receiver
        { emitter.local_endpoint()
        , [&] (gspc::logging::message const& msg)
          {
            received_promise.set_value (msg);
          }
        }
      };

    // Emit the GANTT event (this is what the Agent does)
    emitter.emit_message ({event.encoded(), gantt_log_category});

    // Wait for the message to be received
    BOOST_REQUIRE_EQUAL
      ( received_future.wait_for (std::chrono::milliseconds (500))
      , std::future_status::ready
      );

    auto const received_message {received_future.get()};

    // Verify category
    BOOST_REQUIRE_EQUAL (received_message._category, gantt_log_category);

    // Decode the event from the received message
    auto const decoded_event {NotificationEvent {received_message._content}};

    // Verify the decoded event has all the custom annotations
    BOOST_REQUIRE_EQUAL (decoded_event.activity_id(), event.activity_id());
    BOOST_REQUIRE_EQUAL
      (decoded_event.activity_name(), event.activity_name());
    BOOST_REQUIRE (decoded_event.activity_state() == state);

    auto const& colors {decoded_event.colors_for_state()};
    BOOST_REQUIRE_EQUAL
      ( colors.at (NotificationEvent::STATE_STARTED)
      , "#ff0000"
      );
    BOOST_REQUIRE_EQUAL
      ( colors.at (NotificationEvent::STATE_FINISHED)
      , "#00ff00"
      );

    BOOST_REQUIRE (decoded_event.tooltip().has_value());
    BOOST_REQUIRE_EQUAL (*decoded_event.tooltip(), "\"Custom logging test\"");
  }

  BOOST_AUTO_TEST_CASE (to_string_returns_correct_state_names)
  {
    BOOST_REQUIRE_EQUAL
      ( NotificationEvent::to_string (NotificationEvent::STATE_STARTED)
      , "started"
      );
    BOOST_REQUIRE_EQUAL
      ( NotificationEvent::to_string (NotificationEvent::STATE_FINISHED)
      , "finished"
      );
    BOOST_REQUIRE_EQUAL
      ( NotificationEvent::to_string (NotificationEvent::STATE_FAILED)
      , "failed"
      );
    BOOST_REQUIRE_EQUAL
      ( NotificationEvent::to_string (NotificationEvent::STATE_CANCELED)
      , "canceled"
      );
  }

  BOOST_AUTO_TEST_CASE (multiple_gantt_events_with_different_states)
  {
    // Simulate a complete job lifecycle with custom annotations
    auto const activity
      { activity_with_monitor_annotations
        ( "0xFF0000"   // started: red
        , "0x00FF00"   // finished: green
        , "0x0000FF"   // failed: blue
        , "0xFFFF00"   // canceled: yellow
        , "\"Lifecycle test\""
        , std::invoke (random_port_name)
        , std::invoke (random_input_value)
        )
      };
    auto const components {std::invoke (random_components)};
    auto const activity_id {std::invoke (random_activity_id)};

    auto emitter {gspc::logging::stream_emitter{}};

    // Sink that collects all messages
    auto collected_messages {std::vector<gspc::logging::message>{}};
    auto messages_guard {std::mutex{}};
    auto all_messages_arrived {std::condition_variable{}};
    auto const expected_number_of_messages {std::size_t {2}};

    auto receiver
      { gspc::logging::stream_receiver
        { emitter.local_endpoint()
        , [&] (gspc::logging::message const& msg)
          {
            auto const lock {std::lock_guard {messages_guard}};

            collected_messages.push_back (msg);

            if (collected_messages.size() >= expected_number_of_messages)
            {
              all_messages_arrived.notify_one();
            }
          }
        }
      };

    // Emit started event
    {
      auto const started_event
        { NotificationEvent
          { components
          , activity_id
          , NotificationEvent::STATE_STARTED
          , activity
          }
        };
      emitter.emit_message ({started_event.encoded(), gantt_log_category});
    }

    // Emit finished event
    {
      auto const finished_event
        { NotificationEvent
          { components
          , activity_id
          , NotificationEvent::STATE_FINISHED
          , activity
          }
        };
      emitter.emit_message ({finished_event.encoded(), gantt_log_category});
    }

    // Wait for both messages
    {
      auto lock {std::unique_lock<std::mutex> {messages_guard}};

      auto const received
        { all_messages_arrived.wait_for
            ( lock
            , std::chrono::milliseconds (500)
            , [&]
              {
                return collected_messages.size()
                    >= expected_number_of_messages
                  ;
              }
            )
        };

      BOOST_REQUIRE (received);
    }

    // Verify both messages were received with correct states
    // Note: Messages may arrive out of order, so find by state
    BOOST_REQUIRE_EQUAL (collected_messages.size(), 2);

    auto const find_by_state
      { [&] (NotificationEvent::state_t state)
        {
          auto const message
            { std::find_if
                ( collected_messages.begin()
                , collected_messages.end()
                , [&] (gspc::logging::message const& msg)
                  {
                    return NotificationEvent {msg._content}
                      .activity_state() == state;
                  }
                )
            };

          BOOST_REQUIRE (message != collected_messages.end());

          return NotificationEvent {message->_content};
        }
      };

    auto const decoded_started
      { find_by_state (NotificationEvent::STATE_STARTED)
      };
    BOOST_REQUIRE_EQUAL
      ( decoded_started.colors_for_state()
          .at (NotificationEvent::STATE_STARTED)
      , "#ff0000"
      );

    auto const decoded_finished
      { find_by_state (NotificationEvent::STATE_FINISHED)
      };
    BOOST_REQUIRE_EQUAL
      ( decoded_finished.colors_for_state()
          .at (NotificationEvent::STATE_FINISHED)
      , "#00ff00"
      );
  }
}
