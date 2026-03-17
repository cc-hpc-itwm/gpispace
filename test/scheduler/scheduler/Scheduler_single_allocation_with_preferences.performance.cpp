// Copyright (C) 2019-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/scheduler/daemon/WorkerManager.hpp>
#include <gspc/scheduler/daemon/scheduler/Scheduler.hpp>
#include <gspc/scheduler/daemon/scheduler/SingleAllocationScheduler.hpp>
#include <test/scheduler/scheduler/utils.hpp>
#include <gspc/scheduler/types.hpp>

#include <gspc/we/type/Requirement.hpp>
#include <gspc/we/type/schedule_data.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/set.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_maximum_running_time.hpp>

#include <optional>
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
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
    return gspc::testing::random_integral<unsigned long>();
  }

  const double computational_cost = 1.0;

  gspc::we::type::Requirements_and_preferences require
    ( std::string const& capability
    , gspc::we::type::Preferences const& preferences
    )
  {
    return { {gspc::we::type::Requirement (capability)}
           , gspc::we::type::schedule_data()
           , gspc::we::type::null_transfer_cost
           , computational_cost
           , 0
           , preferences
           };
  }
}

struct fixture_add_new_workers
{
  fixture_add_new_workers()
    : _worker_manager()
    , _scheduler
        ( std::make_unique<gspc::scheduler::daemon::SingleAllocationScheduler>
            ( std::bind ( &fixture_add_new_workers::requirements_and_preferences
                        , this
                        , std::placeholders::_1
                        )
            , _worker_manager
            )
        )
  {}

  gspc::scheduler::daemon::WorkerManager _worker_manager;
  std::unique_ptr<gspc::scheduler::daemon::Scheduler> _scheduler;

  void add_job ( gspc::scheduler::job_id_t const& job_id
               , gspc::we::type::Requirements_and_preferences const& reqs
               )
  {
    if (!_requirements_and_preferences.emplace (job_id, reqs).second)
    {
      throw std::runtime_error ("added job twice");
    }
  }

  gspc::we::type::Requirements_and_preferences requirements_and_preferences
    ( gspc::scheduler::job_id_t id
    )
  {
    return _requirements_and_preferences.find (id)->second;
  }

  void add_new_workers
    ( std::unordered_set<std::string> const& cpbnames
    , unsigned int n
    )
  {
    gspc::testing::random<std::string> const random_string;
    gspc::testing::random<unsigned short> const random_ushort;

    while (n --> 0)
    {
      auto const worker (utils::random_peer_name());
      gspc::scheduler::Capabilities cpbset;
      for (std::string const& capability_name : cpbnames)
      {
        cpbset.emplace (capability_name);
      }

      auto const hostname {random_string()};

      std::lock_guard<std::mutex> lock (_worker_manager._mutex);
      _worker_manager.add_worker
        ( worker
        , cpbset
        , random_ulong()
        , hostname
        , gspc::com::p2p::address_t
            { gspc::com::host_t {hostname}
            , gspc::com::port_t {random_ushort()}
            }
        );
    }
  }

  void request_scheduling()
  {
    _scheduler->assign_jobs_to_workers();
  }

private:
  std::map<gspc::scheduler::job_id_t, gspc::we::type::Requirements_and_preferences>
    _requirements_and_preferences;
};

BOOST_FIXTURE_TEST_CASE
  ( performance_single_allocation_scheduler_with_preferences
  , fixture_add_new_workers
  )
{

  gspc::testing::unique_random<gspc::scheduler::job_id_t> job_name_pool;
  gspc::testing::unique_random<std::string> capability_pool;

  std::string const common_capability (capability_pool());

  gspc::we::type::Preferences const preferences
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

  GSPC_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (80))
  {
    for (unsigned int i {0}; i < num_tasks; i++)
    {
      gspc::scheduler::job_id_t const task (job_name_pool());
      add_job (task, require (common_capability, preferences));

      _scheduler->submit_job (task);
      request_scheduling();
    }
  };
}
