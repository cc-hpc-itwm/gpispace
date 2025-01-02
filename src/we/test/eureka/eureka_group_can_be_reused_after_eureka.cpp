// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <we/test/eureka/Fixture.hpp>
#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <map>
#include <string>
#include <utility>

BOOST_FIXTURE_TEST_CASE
  ( we_eureka_eureka_group_can_be_reused_after_eureka
  , gspc::we::test::eureka::Fixture
  )
{
  auto const worker_per_node (random.between (0, 10));

  WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT
    ( eureka_group_can_be_reused
    , "work:" + std::to_string (worker_per_node)
    );

  auto const number_of_eureka (random.between (2, 5));

  WE_TEST_EUREKA_PUT_TOKEN ("num_eureka", number_of_eureka);

  TasksByEurekaGroup expected_tasks;

  auto const eureka_group (random.unique_eureka_group());
  auto const eureka_values (random.eureka_values (number_of_eureka));
  unsigned int total_number_of_tasks (0);

  for (unsigned int e (0); e < number_of_eureka; ++e)
  {
    auto const number_of_tasks
      (random.between (0, worker_per_node * NUMBER_OF_NODES()));

    for (unsigned int t (0); t < number_of_tasks; ++t, ++total_number_of_tasks)
    {
      auto const task (random.unique_task());
      expected_tasks[eureka_group].emplace (task);

      WE_TEST_EUREKA_PUT_TOKEN ("task", task);
      WE_TEST_EUREKA_PUT_TOKEN ("eureka_group", eureka_group);
      WE_TEST_EUREKA_PUT_TOKEN ("sleep_for", random.sleep_time());
    }

    jobserver.wait
      (total_number_of_tasks, expected_tasks, JobServer::Running{});

    WE_TEST_EUREKA_PUT_TOKEN ("eureka", eureka_values.at (e));
    WE_TEST_EUREKA_PUT_TOKEN ("eureka_group", eureka_group);

    jobserver.wait
      (total_number_of_tasks, expected_tasks, JobServer::ExitedOrCancelled{});
  }

  auto const result (WE_TEST_EUREKA_GET_RESULT());

  std::multimap<std::string, pnet::type::value::value_type> expected
    {{"awaited_eureka", 0u}}
  ;

  for (auto eureka_value : eureka_values)
  {
    expected.insert (std::make_pair ("result", eureka_value));
  }

  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);

  BOOST_REQUIRE (jobserver.tasks (JobServer::Cancelled{}).empty());
}
