// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <we/test/eureka/Fixture.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <string>

BOOST_FIXTURE_TEST_CASE
  ( we_eureka_multiple_functions_in_the_same_group
  , gspc::we::test::eureka::Fixture
  )
{
  auto const worker_per_node_per_function (random.between (1, 25));

  WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT
    ( multiple_functions_in_the_same_group
    , "work:" + std::to_string (3 * worker_per_node_per_function)
    );

  TasksByEurekaGroup expected_running;
  TasksByEurekaGroup expected_exited_or_cancelled;
  TasksByEurekaGroup expected_cancelled;

  auto const eureka_group (random.unique_eureka_group());
  WE_TEST_EUREKA_PUT_TOKEN ("eureka_group", eureka_group);

  unsigned int total_number_of_tasks (0);
  unsigned int const number_of_tasks_per_function
    (worker_per_node_per_function * NUMBER_OF_NODES());

  for ( unsigned int t (0)
      ; t < number_of_tasks_per_function
      ; ++t, total_number_of_tasks += 3
      )
  {
    for (std::string place : {"sleep_for_A", "sleep_for_B"})
    {
      auto const task (random.unique_task());
      expected_running[eureka_group].emplace (task);
      expected_exited_or_cancelled[eureka_group].emplace (task);

      WE_TEST_EUREKA_PUT_TOKEN ("task", task);
      WE_TEST_EUREKA_PUT_TOKEN (place, random.sleep_time());
    }

    jobserver.wait
      (total_number_of_tasks + 2, expected_running, JobServer::Running{});

    for (std::string place : {"loop"})
    {
      auto const task (random.unique_task());
      expected_running[eureka_group].emplace (task);
      expected_cancelled[eureka_group].emplace (task);

      WE_TEST_EUREKA_PUT_TOKEN ("task", task);
      WE_TEST_EUREKA_PUT_TOKEN (place, we::type::literal::control());
    }
  }

  jobserver.wait
    (total_number_of_tasks, expected_running, JobServer::Running{});

  auto const eureka_value (random.eureka_value());

  WE_TEST_EUREKA_PUT_TOKEN ("eureka", eureka_value);

  jobserver.wait
    ( 2 * number_of_tasks_per_function
    , expected_exited_or_cancelled
    , JobServer::ExitedOrCancelled{}
    );
  jobserver.wait
    ( number_of_tasks_per_function
    , expected_cancelled
    , JobServer::Cancelled{}
    );

  auto const result (WE_TEST_EUREKA_GET_RESULT());

  decltype (result) const expected {{"result", eureka_value}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}
