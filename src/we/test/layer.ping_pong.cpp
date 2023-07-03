// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <testing/source_directory.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <we/layer.hpp>
#include <we/type/Activity.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>

#include <array>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <random>
#include <utility>

namespace
{
  [[noreturn]] void disallow (std::string what)
  {
    throw std::runtime_error ("disallowed function called: " + what);
  }
}

BOOST_AUTO_TEST_CASE (emulate_share_example_ping_pong)
{
  unsigned long const N (1 << 15);

  ::boost::program_options::options_description options_description;
  options_description.add (test::options::source_directory());
  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        )
    . options (options_description)
    . run()
    , vm
    );

  ::boost::filesystem::path const xpnet_path
    ( test::source_directory (vm)
    / "share" / "doc" / "example" / "ping-pong" / "ping-pong.xpnet"
    );

  std::mt19937 random_extraction_engine;
  std::atomic<std::size_t> current_id (0);

  std::mutex current_state_guard;
  std::condition_variable current_activity_changed;
  ::boost::optional<std::pair<we::layer::id_type, we::type::Activity>>
    current_activity;

  std::mutex finished_guard;
  std::condition_variable finished_changed;
  bool finished (false);

  we::layer layer
    ( [&current_state_guard, &current_activity, &current_activity_changed]
        (we::layer::id_type id, we::type::Activity activity)
      {
        std::lock_guard<std::mutex> const _ (current_state_guard);
        current_activity
          = std::make_pair (std::move (id), std::move (activity));
        current_activity_changed.notify_one();
      }
    , std::bind (&disallow, "cancel")
    , [&finished_guard, &finished_changed, &finished]
        (we::layer::id_type, we::type::Activity)
      {
        std::lock_guard<std::mutex> const _ (finished_guard);
        finished = true;
        finished_changed.notify_one();
      }
    , std::bind (&disallow, "failed")
    , std::bind (&disallow, "canceled")
    , std::bind (&disallow, "token_put")
    , std::bind (&disallow, "workflow_response")
    , [&current_id]
      {
        return std::to_string (++current_id);
      }
    , random_extraction_engine
    );

  {
    xml::parse::state::type parser_state;
    xml::parse::type::function_type parsed
      (xml::parse::just_parse (parser_state, xpnet_path));
    xml::parse::post_processing_passes (parsed, &parser_state);
    we::type::Activity activity
      (xml::parse::xml_to_we (parsed, parser_state));
    activity.add_input ("n", N);
    layer.submit (we::layer::id_type(), activity);
  }

  std::array<std::string, 2> const names {{"ping", "pong"}};
  std::size_t current (0);

  for (std::size_t reactions (0); reactions < 2 * N; ++reactions)
  {
    std::unique_lock<std::mutex> lock (current_state_guard);
    current_activity_changed.wait (lock, [&] { return !!current_activity; });

    we::type::Activity activity (std::move (current_activity->second));

    BOOST_REQUIRE_EQUAL (activity.name(), names[current]);
    current = !current;

    activity.add_output_TESTING_ONLY
      ( "seq"
      , activity.input().front().first
      );
    layer.finished ( std::move (current_activity->first)
                   , std::move (activity)
                   );

    current_activity = ::boost::none;
  }

  {
    std::unique_lock<std::mutex> lock (finished_guard);
    finished_changed.wait (lock, [&] { return finished; });
  }
}
