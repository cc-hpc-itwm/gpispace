// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <we/test/eureka/Fixture.hpp>
#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/printer/unordered_map.hpp>
#include <util-generic/testing/printer/unordered_set.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <iterator>
#include <map>
#include <numeric>
#include <string>
#include <unordered_set>
#include <vector>

BOOST_FIXTURE_TEST_CASE
  ( we_eureka_multiple_groups
  , gspc::we::test::eureka::Fixture
  )
{
  auto const number_of_groups (random.between (2, 10));
  auto const worker_per_node_per_group (random.between (0, 15));

  WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT
    ( eureka_group_can_be_reused
    , "work:" + std::to_string (number_of_groups * worker_per_node_per_group)
    );

  WE_TEST_EUREKA_PUT_TOKEN ("num_eureka", number_of_groups);

  TasksByEurekaGroup expected_tasks;
  using Tasks = std::unordered_set<Task>;
  std::vector<Tasks> expected_tasks_by_eureka_group (number_of_groups);

  auto const eureka_groups (random.unique_eureka_groups (number_of_groups));
  unsigned int total_number_of_tasks (0);

  for (unsigned int g (0); g < number_of_groups; ++g)
  {
    auto const number_of_tasks
      (random.between (0, worker_per_node_per_group * NUMBER_OF_NODES()));

    for (unsigned int t (0); t < number_of_tasks; ++t, ++total_number_of_tasks)
    {
      auto const task (random.unique_task());
      expected_tasks[eureka_groups.at (g)].emplace (task);
      expected_tasks_by_eureka_group[g].emplace (task);

      WE_TEST_EUREKA_PUT_TOKEN ("task", task);
      WE_TEST_EUREKA_PUT_TOKEN ("eureka_group", eureka_groups.at (g));
      WE_TEST_EUREKA_PUT_TOKEN ("sleep_for", random.sleep_time());
    }

    jobserver.wait
      (total_number_of_tasks, expected_tasks, JobServer::Running());
  }

  auto const eureka_values (random.eureka_values (number_of_groups));

  std::vector<unsigned int> indices (number_of_groups);
  std::iota (std::begin (indices), std::end (indices), 0);
  std::shuffle ( std::begin (indices), std::end (indices)
               , fhg::util::testing::detail::GLOBAL_random_engine()
               );

  for (auto g : indices)
  {
    WE_TEST_EUREKA_PUT_TOKEN ("eureka", eureka_values.at (g));
    WE_TEST_EUREKA_PUT_TOKEN ("eureka_group", eureka_groups.at (g));

    jobserver.wait
      ( expected_tasks_by_eureka_group[g].size()
      , eureka_groups.at (g)
      , JobServer::ExitedOrCancelled{}
      );

    FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION
      ( expected_tasks_by_eureka_group[g]
      , jobserver.tasks (JobServer::ExitedOrCancelled{})[eureka_groups.at (g)]
      );
  }

  jobserver.wait
    (total_number_of_tasks, expected_tasks, JobServer::ExitedOrCancelled{});

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
