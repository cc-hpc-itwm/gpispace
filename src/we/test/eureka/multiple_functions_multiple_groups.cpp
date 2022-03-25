// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <boost/test/unit_test.hpp>

#include <we/type/value/boost/test/printer.hpp>
#include <we/test/eureka/Fixture.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <string>

BOOST_FIXTURE_TEST_CASE
  ( we_eureka_multiple_functions_multiple_groups
  , gspc::we::test::eureka::Fixture
  )
{
  auto const worker_per_node_per_function (random.between (1, 25));

  WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT
    ( multiple_functions_multiple_groups
    , "work:" + std::to_string (3 * worker_per_node_per_function)
    );

  TasksByEurekaGroup expected_tasks;

  auto const eureka_group_A_ (random.unique_eureka_group());
  auto const eureka_group__B (random.unique_eureka_group());
  auto const eureka_group_AB (random.unique_eureka_group());

  unsigned int total_number_of_tasks (0);
  unsigned int const number_of_tasks_per_function
    (worker_per_node_per_function * NUMBER_OF_NODES());

  for ( unsigned int t (0)
      ; t < number_of_tasks_per_function
      ; ++t, total_number_of_tasks += 3
      )
  {
    auto const task_A_ (random.unique_task());
    expected_tasks[eureka_group_A_].emplace (task_A_);

    WE_TEST_EUREKA_PUT_TOKEN ("task_A", task_A_);
    WE_TEST_EUREKA_PUT_TOKEN ("eureka_group_A", eureka_group_A_);
    WE_TEST_EUREKA_PUT_TOKEN ("sleep_for_A", random.sleep_time());

    auto const task__B (random.unique_task());
    expected_tasks[eureka_group__B].emplace (task__B);

    WE_TEST_EUREKA_PUT_TOKEN ("task_B", task__B);
    WE_TEST_EUREKA_PUT_TOKEN ("eureka_group_B", eureka_group__B);
    WE_TEST_EUREKA_PUT_TOKEN ("sleep_for_B", random.sleep_time());

    jobserver.wait
      (total_number_of_tasks + 2, expected_tasks, JobServer::Running{});

    auto const task_AB (random.unique_task());
    expected_tasks[eureka_group_AB].emplace (task_AB);

    if (t % 2)
    {
      WE_TEST_EUREKA_PUT_TOKEN ("task_A", task_AB);
      WE_TEST_EUREKA_PUT_TOKEN ("eureka_group_A", eureka_group_AB);
      WE_TEST_EUREKA_PUT_TOKEN ("sleep_for_A", random.sleep_time());
    }
    else
    {
      WE_TEST_EUREKA_PUT_TOKEN ("task_B", task_AB);
      WE_TEST_EUREKA_PUT_TOKEN ("eureka_group_B", eureka_group_AB);
      WE_TEST_EUREKA_PUT_TOKEN ("sleep_for_B", random.sleep_time());
    }
  }

  jobserver.wait
    (total_number_of_tasks, expected_tasks, JobServer::Running{});

  auto const eureka_A__value (random.eureka_value());
  WE_TEST_EUREKA_PUT_TOKEN ("eureka", eureka_A__value);
  WE_TEST_EUREKA_PUT_TOKEN ("eureka_group", eureka_group_A_);

  auto const eureka__B_value (random.eureka_value());
  WE_TEST_EUREKA_PUT_TOKEN ("eureka", eureka__B_value);
  WE_TEST_EUREKA_PUT_TOKEN ("eureka_group", eureka_group__B);

  auto const eureka_AB_value (random.eureka_value());
  WE_TEST_EUREKA_PUT_TOKEN ("eureka", eureka_AB_value);
  WE_TEST_EUREKA_PUT_TOKEN ("eureka_group", eureka_group_AB);

  for (auto g : {eureka_group_A_, eureka_group__B, eureka_group_AB})
  {
    jobserver.wait
      (number_of_tasks_per_function, g, JobServer::ExitedOrCancelled{});
  }
  jobserver.wait
    ( total_number_of_tasks
    , expected_tasks
    , JobServer::ExitedOrCancelled{}
    );

  auto const result (WE_TEST_EUREKA_GET_RESULT());

  decltype (result) const expected
    { {"result", eureka_A__value}
    , {"result", eureka__B_value}
    , {"result", eureka_AB_value}
    }
  ;
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);

  BOOST_REQUIRE (jobserver.tasks (JobServer::Cancelled{}).empty());
}
