// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <we/test/eureka/Fixture.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <fmt/core.h>

#include <string>

BOOST_FIXTURE_TEST_CASE
  ( we_eureka_multi_mod_one_worker_type
  , gspc::we::test::eureka::Fixture
  )
{
  auto const worker_per_node (random.between (1, 100));

  auto const topology { fmt::format ("{}:{}"
                                    , random.bit() ? "A" : "B"
                                    , worker_per_node
                                    )
                      };

  WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT
    (multi_mod, topology);

  TasksByEurekaGroup expected_tasks;

  auto const eureka_group (random.unique_eureka_group());
  WE_TEST_EUREKA_PUT_TOKEN ("eureka_group", eureka_group);

  auto const number_of_tasks
    (random.between (0, worker_per_node * NUMBER_OF_NODES()));

  for (unsigned int t (0); t < number_of_tasks; ++t)
  {
    auto const task (random.unique_task());
    expected_tasks[eureka_group].emplace (task);

    WE_TEST_EUREKA_PUT_TOKEN ("task", task);
    WE_TEST_EUREKA_PUT_TOKEN ("sleep_for", random.sleep_time());
  }

  jobserver.wait (number_of_tasks, expected_tasks, JobServer::Running{});

  auto const eureka_value (random.eureka_value());

  WE_TEST_EUREKA_PUT_TOKEN ("eureka", eureka_value);

  jobserver.wait
    (number_of_tasks, expected_tasks, JobServer::ExitedOrCancelled{});

  auto const result (WE_TEST_EUREKA_GET_RESULT());

  decltype (result) const expected {{"result", eureka_value}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);

  BOOST_REQUIRE (jobserver.tasks (JobServer::Cancelled{}).empty());
}

BOOST_FIXTURE_TEST_CASE
  ( we_eureka_multi_mod_two_worker_types
  , gspc::we::test::eureka::Fixture
  )
{
  auto const worker_per_node (random.between (1, 100));

  WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT
    ( multi_mod
    , "A:" + std::to_string (worker_per_node)
    + " B:" + std::to_string (worker_per_node)
    );

  TasksByEurekaGroup expected_tasks;

  auto const eureka_group (random.unique_eureka_group());
  WE_TEST_EUREKA_PUT_TOKEN ("eureka_group", eureka_group);

  auto const number_of_tasks
    (random.between (0, 2 * worker_per_node * NUMBER_OF_NODES()));

  for (unsigned int t (0); t < number_of_tasks; ++t)
  {
    auto const task (random.unique_task());
    expected_tasks[eureka_group].emplace (task);

    WE_TEST_EUREKA_PUT_TOKEN ("task", task);
    WE_TEST_EUREKA_PUT_TOKEN ("sleep_for", random.sleep_time());
  }

  jobserver.wait (number_of_tasks, expected_tasks, JobServer::Running{});

  auto const eureka_value (random.eureka_value());

  WE_TEST_EUREKA_PUT_TOKEN ("eureka", eureka_value);

  jobserver.wait
    (number_of_tasks, expected_tasks, JobServer::ExitedOrCancelled{});

  auto const result (WE_TEST_EUREKA_GET_RESULT());

  decltype (result) const expected {{"result", eureka_value}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);

  BOOST_REQUIRE (jobserver.tasks (JobServer::Cancelled{}).empty());
}
