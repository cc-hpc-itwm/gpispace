// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <we/test/eureka/Fixture.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <string>

BOOST_FIXTURE_TEST_CASE
  ( we_eureka_pending_tasks_are_removed
  , gspc::we::test::eureka::Fixture
  )
{
  auto const worker_per_node (random.between (0, 100));

  WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT
    ( pending_tasks_are_removed
    , "work:" + std::to_string (worker_per_node)
    );

  TasksByEurekaGroup expected_tasks;

  auto const eureka_group (random.unique_eureka_group());
  WE_TEST_EUREKA_PUT_TOKEN ("eureka_group", eureka_group);

  auto const number_of_workers (worker_per_node * NUMBER_OF_NODES());

  for (unsigned int t (0); t < number_of_workers; ++t)
  {
    auto const task (random.unique_task());
    expected_tasks[eureka_group].emplace (task);

    WE_TEST_EUREKA_PUT_TOKEN ("task", task);
  }

  jobserver.wait (number_of_workers, expected_tasks, JobServer::Running{});

  auto const number_of_pending_tasks (random.between (0, 100));

  // all workers have a task running, new tasks can not start
  for (unsigned int t (0); t < number_of_pending_tasks; ++t)
  {
    auto const task (random.unique_task());
    // pending tasks are removed with no observation of neither start
    // nor cancel

    WE_TEST_EUREKA_PUT_TOKEN ("task", task);
  }

  auto const eureka_value (random.eureka_value());

  WE_TEST_EUREKA_PUT_TOKEN ("eureka", eureka_value);

  jobserver.wait (number_of_workers, expected_tasks, JobServer::Cancelled{});

  auto const result (WE_TEST_EUREKA_GET_RESULT());

  decltype (result) const expected {{"result", eureka_value}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);

  BOOST_REQUIRE (jobserver.tasks (JobServer::ExitedOrCancelled{}).empty());
}
