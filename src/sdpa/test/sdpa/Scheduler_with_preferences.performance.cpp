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

#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/set.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_maximum_running_time.hpp>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>

#include <algorithm>
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

  Requirements_and_preferences require
    ( std::string const& capability
    , Preferences const& preferences
    )
  {
    return { {we::type::requirement_t (capability, true)}
           , we::type::schedule_data()
           , null_transfer_cost
           , computational_cost
           , 0
           , preferences
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

  void add_job ( const sdpa::job_id_t& job_id
               , const Requirements_and_preferences& reqs
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
        cpbset.emplace (capability_name, worker);
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
  (performance_scheduling_tasks_with_preferences, fixture_add_new_workers)
{

  fhg::util::testing::unique_random<sdpa::job_id_t> job_name_pool;
  fhg::util::testing::unique_random<std::string> capability_pool;

  std::string const common_capability (capability_pool());

  Preferences const preferences
    {capability_pool(), capability_pool(), capability_pool()};

  unsigned int const num_workers (2000);
  unsigned int const num_tasks (15000);
  unsigned int const n (num_workers/3);

  add_new_workers ( {common_capability, *preferences.begin()}
                  , n
                  );
  add_new_workers ( {common_capability, *std::next (preferences.begin(), 1)}
                  , n
                  );
  add_new_workers ( {common_capability, *std::next (preferences.begin(), 2)}
                  , num_workers - 2*n
                  );

  FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (80))
  {
    for (unsigned int i {0}; i < num_tasks; i++)
    {
      sdpa::job_id_t const task (job_name_pool());
      add_job (task, require (common_capability, preferences));

      _scheduler.submit_job (task);
      request_scheduling();
    }
  };
}
