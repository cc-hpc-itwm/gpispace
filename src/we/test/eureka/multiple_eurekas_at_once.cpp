// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <we/test/eureka/Fixture.hpp>
#include <we/type/value/boost/test/printer.hpp>
#include <we/type/value/wrap.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <boost/range/adaptor/indexed.hpp>

#include <set>
#include <string>

BOOST_FIXTURE_TEST_CASE
  ( we_eureka_multiple_eurekas_at_once
  , gspc::we::test::eureka::Fixture
  )
{
  auto const number_of_groups (random.between (2, 10));
  auto const worker_per_node_per_group (random.between (0, 15));

  WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT
    ( multiple_eurekas_at_once
    , "work:" + std::to_string (number_of_groups * worker_per_node_per_group)
    );

  TasksByEurekaGroup expected_tasks;

  auto const eureka_groups (random.unique_eureka_groups (number_of_groups));
  unsigned int total_number_of_tasks (0);

  for (auto group : eureka_groups | ::boost::adaptors::indexed (0))
  {
    auto const number_of_tasks
      (random.between (0, worker_per_node_per_group * NUMBER_OF_NODES()));

    for (unsigned int t (0); t < number_of_tasks; ++t, ++total_number_of_tasks)
    {
      auto const task (random.unique_task());
      expected_tasks[group.value()].emplace (task);

      WE_TEST_EUREKA_PUT_TOKEN ("task", task);
      WE_TEST_EUREKA_PUT_TOKEN ("eureka_group", group.value());
      WE_TEST_EUREKA_PUT_TOKEN ("sleep_for", random.sleep_time());
    }

    jobserver.wait
      (total_number_of_tasks, expected_tasks, JobServer::Running());
  }

  auto const eurekas
    ( pnet::type::value::wrap
        (std::set<EurekaGroup> (eureka_groups.cbegin(), eureka_groups.cend()))
    );

  WE_TEST_EUREKA_PUT_TOKEN ("eureka", eurekas);

  jobserver.wait
    (total_number_of_tasks, expected_tasks, JobServer::Running());

  auto const result (WE_TEST_EUREKA_GET_RESULT());

  decltype (result) const expected {{"result", eurekas}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);

  BOOST_REQUIRE (jobserver.tasks (JobServer::Cancelled{}).empty());
}
