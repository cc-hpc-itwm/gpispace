// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <we/type/Requirement.hpp>
#include <we/type/schedule_data.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/set.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_maximum_running_time.hpp>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>

#include <chrono>
#include <functional>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
  unsigned long random_ulong()
  {
    return fhg::util::testing::random_integral<unsigned long>();
  }

  const double computational_cost = 1.0;

  Requirements_and_preferences require (std::string const& capability)
  {
    return { {we::type::Requirement (capability)}
           , we::type::schedule_data()
           , null_transfer_cost
           , computational_cost
           , 0
           , {}
           };
  }
}

struct fixture_add_new_workers
{
  fixture_add_new_workers()
    : _scheduler
        (std::bind ( &fixture_add_new_workers::requirements_and_preferences
                   , this
                   , std::placeholders::_1
                   )
        )
  {}

  sdpa::daemon::CoallocationScheduler _scheduler;

  void add_job ( sdpa::job_id_t const& job_id
               , Requirements_and_preferences const& reqs
               )
  {
    if (!_requirements_and_preferences.emplace (job_id, reqs).second)
    {
      throw std::runtime_error ("added job twice");
    }
  }

  Requirements_and_preferences requirements_and_preferences (sdpa::job_id_t id)
  {
    return _requirements_and_preferences.find (id)->second;
  }

  void add_new_workers
    ( std::unordered_set<std::string> const& cpbnames
    , unsigned int n
    )
  {
    while (n --> 0)
    {
      auto const worker (utils::random_peer_name());
      sdpa::capabilities_set_t cpbset;
      for (std::string const& capability_name : cpbnames)
      {
        cpbset.emplace (capability_name);
      }

      _scheduler.add_worker ( worker
                            , cpbset
                            , random_ulong()
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );
    }
  }

  void request_scheduling()
  {
    _scheduler.assign_jobs_to_workers();
    _scheduler.steal_work();
  }

private:
  std::map<sdpa::job_id_t, Requirements_and_preferences>
    _requirements_and_preferences;
};

BOOST_FIXTURE_TEST_CASE
  (performance_scheduling_tasks_with_no_preferences, fixture_add_new_workers)
{
  using fhg::util::testing::random_integral;

  fhg::util::testing::unique_random<sdpa::job_id_t> job_name_pool;
  fhg::util::testing::unique_random<std::string> capability_pool;

  std::unordered_set<std::string> const capabilities
    {capability_pool(), capability_pool()};

  unsigned int const num_workers (2000);
  unsigned int const num_tasks (15000);

  add_new_workers (capabilities, num_workers);

  FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (60))
  {
    for (unsigned int i {0}; i < num_tasks; i++)
    {
      sdpa::job_id_t const task (job_name_pool());
      add_job
        ( task
        , require (*std::next ( capabilities.begin()
                              , random_integral<unsigned int>() % 2
                              )
                  )
        );

      _scheduler.submit_job (task);
      request_scheduling();
    }
  };
}
