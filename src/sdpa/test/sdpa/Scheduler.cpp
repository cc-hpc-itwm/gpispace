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

#include <sdpa/daemon/Implementation.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/WorkerSet.hpp>
#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
#include <sdpa/daemon/scheduler/CostAwareWithWorkStealingStrategy.hpp>
#include <sdpa/daemon/scheduler/GreedyScheduler.hpp>
#include <sdpa/daemon/scheduler/Scheduler.hpp>
#include <sdpa/daemon/scheduler/SingleAllocationScheduler.hpp>
#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <we/type/Requirement.hpp>
#include <we/type/schedule_data.hpp>

#include <fhg/util/next.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/set.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/iterator/transform_iterator.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <chrono>
#include <functional>
#include <iterator>
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

#define MULTIPLE_FIXTURES_TEST_CASE(NAME, TPARAM, ...) \
  typedef ::boost::mpl::vector<__VA_ARGS__> NAME##_fixtures; \
  BOOST_FIXTURE_TEST_CASE_TEMPLATE(NAME, TPARAM, NAME##_fixtures, TPARAM)

#define CHECK_ALL_WORKERS_HAVE_AT_LEAST_ONE_TASK_ASSIGNED(_workers) \
  do                                                                \
  {                                                                 \
    for (unsigned int i (0); i < _workers.size(); ++i)              \
    {                                                               \
      BOOST_REQUIRE_GT                                              \
        ( count_assigned_jobs                                       \
            (get_current_assignment(), _workers[i])                 \
        , 0                                                         \
        );                                                          \
    }                                                               \
  } while (false)

#define CHECK_ALL_WORKERS_HAVE_AT_MOST_ONE_TASK_ASSIGNED(_workers)  \
  do                                                                \
  {                                                                 \
    for (unsigned int i (0); i < _workers.size(); ++i)              \
    {                                                               \
      BOOST_REQUIRE_GE                                              \
        ( count_assigned_jobs                                       \
            (get_current_assignment(), _workers[i])                 \
        , 0                                                         \
        );                                                          \
                                                                    \
      BOOST_REQUIRE_LE                                              \
        ( count_assigned_jobs                                       \
            (get_current_assignment(), _workers[i])                 \
        , 1                                                         \
        );                                                          \
    }                                                               \
  } while (false)

#define CHECK_EXPECTED_WORKERS_AND_IMPLEMENTATION(_job, _workers, _implementation)  \
  do                                                                                \
  {                                                                                 \
    BOOST_REQUIRE_EQUAL (_workers.size(), 1);                                       \
    BOOST_REQUIRE_EQUAL (this->workers (_job), _workers);                           \
    BOOST_REQUIRE_EQUAL (this->implementation (_job), _implementation);             \
  } while (false)

#define SERVE_JOB_AND_CHECK_EXPECTED(job)     \
  [&job]                                      \
  ( sdpa::daemon::WorkerSet const&            \
  , sdpa::daemon::Implementation const&       \
  , sdpa::job_id_t const& started_job         \
  , std::function<fhg::com::p2p::address_t    \
      (sdpa::worker_id_t const&)>             \
  )                                           \
  {                                           \
    BOOST_REQUIRE_EQUAL (job, started_job);   \
  }

#define EXPECT_NO_JOB_SERVED                  \
  []                                          \
  ( sdpa::daemon::WorkerSet const&            \
  , sdpa::daemon::Implementation const&       \
  , sdpa::job_id_t const&                     \
  , std::function<fhg::com::p2p::address_t    \
      (sdpa::worker_id_t const&)>             \
  )                                           \
  {                                           \
    throw std::runtime_error                  \
      ("No job expected to be served");       \
  }

#define SERVE_JOB_AND_CHECK_IS_ONE_OF(jobs)   \
  [&jobs]                                     \
  ( sdpa::daemon::WorkerSet const&            \
  , sdpa::daemon::Implementation const&       \
  , sdpa::job_id_t const& started_job         \
  , std::function<fhg::com::p2p::address_t    \
      (sdpa::worker_id_t const&)>             \
  )                                           \
  {                                           \
    BOOST_REQUIRE (jobs.count (started_job)); \
  }

namespace
{
  unsigned long random_ulong()
  {
    return fhg::util::testing::random_integral<unsigned long>();
  }

  using set_workers_t = std::set<sdpa::worker_id_t>;
  using set_jobs_t = std::set<sdpa::job_id_t>;
}

namespace sdpa
{
  namespace daemon
  {
    class access_allocation_table_TESTING_ONLY
    {
    public:
      access_allocation_table_TESTING_ONLY (CostAwareWithWorkStealingStrategy& _strategy)
        : _allocation_table (_strategy.allocation_table_)
      {}
      std::map<job_id_t, std::set<worker_id_t>> get_current_assignment() const
      {
        std::map<job_id_t, std::set<worker_id_t>> assignment;
        std::transform
          ( _allocation_table.begin()
          , _allocation_table.end()
          , std::inserter (assignment, assignment.end())
          , [] (auto const& p)
            {
              return std::make_pair (p.first, p.second->workers());
            }
          );

        return assignment;
      }

      sdpa::daemon::WorkerSet workers (sdpa::job_id_t const& job) const
      {
        return _allocation_table.at (job)->workers();
      }

      sdpa::daemon::Implementation implementation (sdpa::job_id_t const& job) const
      {
        return _allocation_table.at (job)->implementation();
      }

    private:
      CostAwareWithWorkStealingStrategy::allocation_table_t& _allocation_table;
    };
  }
}

template <typename Scheduler>
struct fixture_scheduler_and_requirements_and_preferences
{
  fixture_scheduler_and_requirements_and_preferences()
    : _worker_manager()
    , _scheduler
        ( std::make_unique<Scheduler>
            ( std::bind
                ( &fixture_scheduler_and_requirements_and_preferences::requirements_and_preferences
                , this
                , std::placeholders::_1
                )
            , _worker_manager
            )
        )
    , _strategy (dynamic_cast<Scheduler*> (_scheduler.get())->strategy_TESTING_ONLY())
    , _access_allocation_table (_strategy)
  {}

  sdpa::daemon::WorkerManager _worker_manager;
  std::unique_ptr<sdpa::daemon::Scheduler> _scheduler;
  sdpa::daemon::CostAwareWithWorkStealingStrategy& _strategy;
  sdpa::daemon::access_allocation_table_TESTING_ONLY _access_allocation_table;

  std::map<sdpa::job_id_t, std::set<sdpa::worker_id_t>> get_current_assignment() const
  {
    return _access_allocation_table.get_current_assignment();
  }

  sdpa::daemon::WorkerSet workers (sdpa::job_id_t const& job) const
  {
    return _access_allocation_table.workers (job);
  }

  sdpa::daemon::Implementation implementation (sdpa::job_id_t const& job) const
  {
    return _access_allocation_table.implementation (job);
  }

  fhg::util::testing::unique_random<sdpa::job_id_t> job_ids;
  sdpa::job_id_t add_and_enqueue_job
    (Requirements_and_preferences reqs_and_prefs)
  {
    auto const job_id (job_ids());
    _requirements_and_preferences.emplace (job_id, std::move (reqs_and_prefs));
    _scheduler->submit_job (job_id);
    return job_id;
  }

  Requirements_and_preferences requirements_and_preferences (sdpa::job_id_t id)
  {
    return _requirements_and_preferences.find (id)->second;
  }

  std::map<sdpa::job_id_t, Requirements_and_preferences> _requirements_and_preferences;

  long count_assigned_jobs
    ( std::map<sdpa::job_id_t, std::set<sdpa::worker_id_t>> assignment
    , sdpa::worker_id_t const& worker_id
    )
  {
    auto value = [](std::pair<sdpa::job_id_t, std::set<sdpa::worker_id_t>>const& p)
                   {
                     return p.second;
                   };


    return (std::count_if ( ::boost::make_transform_iterator (assignment.begin(), value)
                          , ::boost::make_transform_iterator (assignment.end(), value)
                          , [&worker_id] (std::set<sdpa::worker_id_t> const& v)
                            {
                              return v.count (worker_id);
                            }
                          )
           );
  }

  std::set<sdpa::job_id_t> assigned_tasks (sdpa::worker_id_t const& worker)
  {
    std::set<sdpa::job_id_t> tasks;
    for (auto const& task_and_workers : get_current_assignment())
    {
      if (task_and_workers.second.count (worker))
      {
        tasks.emplace (task_and_workers.first);
      }
    }

    return tasks;
  }

  std::set<sdpa::job_id_t> all_assigned_tasks()
  {
    std::set<sdpa::job_id_t> tasks;
    for (auto const& task_and_workers : get_current_assignment())
    {
      tasks.emplace (task_and_workers.first);
    }

    return tasks;
  }

  void require_worker_and_implementation
    ( sdpa::job_id_t const& job
    , sdpa::worker_id_t const& worker
    , ::boost::optional<std::string> const& impl
    )
  {
    auto const assignment (get_current_assignment());
    BOOST_REQUIRE (assignment.count (job));

    BOOST_REQUIRE_EQUAL (sdpa::daemon::WorkerSet {worker}, workers (job));
    BOOST_REQUIRE_EQUAL (impl, implementation (job));
  }

  void finish_tasks_and_steal_work_when_idle_workers_exist
    ( std::size_t const& num_tasks
    , std::vector<sdpa::worker_id_t> const& test_workers
    )
  {
    unsigned int remaining_tasks (num_tasks);

    std::vector<sdpa::job_id_t> running_tasks;

    while (remaining_tasks > 0)
    {
      _scheduler->assign_jobs_to_workers();

      if (remaining_tasks > test_workers.size())
      {
        CHECK_ALL_WORKERS_HAVE_AT_LEAST_ONE_TASK_ASSIGNED (test_workers);
      }
      else
      {
        CHECK_ALL_WORKERS_HAVE_AT_MOST_ONE_TASK_ASSIGNED (test_workers);
      }

      std::set<sdpa::job_id_t> started_tasks;
      _scheduler->start_pending_jobs
      ( [this, &started_tasks]
        ( sdpa::daemon::WorkerSet const& assigned_workers
        , sdpa::daemon::Implementation const& implementation
        , sdpa::job_id_t const& task
        , std::function<fhg::com::p2p::address_t (sdpa::worker_id_t const&)>
        )
        {
          CHECK_EXPECTED_WORKERS_AND_IMPLEMENTATION
            (task, assigned_workers, implementation);
          started_tasks.emplace (task);
        }
      );

      running_tasks.insert
        (running_tasks.end(), started_tasks.begin(), started_tasks.end());

      // finish arbitrary number of tasks
      std::shuffle ( running_tasks.begin()
                   , running_tasks.end()
                   , fhg::util::testing::detail::GLOBAL_random_engine()
                   );

      auto const num_tasks_to_finish
        ( fhg::util::testing::random<std::size_t>{}
            (std::min (test_workers.size(), running_tasks.size()), 1)
        );

      for (unsigned int i (0) ; i < num_tasks_to_finish; ++i)
      {
        _scheduler->release_reservation (running_tasks.back());
        running_tasks.pop_back();
        remaining_tasks--;
      }
    }
  }

  void finish_tasks_assigned_to_worker_and_steal_work
    ( sdpa::worker_id_t const& worker
    , std::vector<sdpa::worker_id_t> const& test_workers
    )
  {
    auto worker_tasks (assigned_tasks (worker));
    BOOST_REQUIRE (!worker_tasks.empty());

    // finish all tasks assigned to the worker given as parameter
    while (!worker_tasks.empty())
    {
      std::set<sdpa::job_id_t> started_tasks;
      _scheduler->start_pending_jobs
        ( [this, &started_tasks]
          ( sdpa::daemon::WorkerSet const& assigned_workers
          , sdpa::daemon::Implementation const& implementation
          , sdpa::job_id_t const& task
          , std::function<fhg::com::p2p::address_t (sdpa::worker_id_t const&)>
          )
          {
            CHECK_EXPECTED_WORKERS_AND_IMPLEMENTATION
              (task, assigned_workers, implementation);
            started_tasks.emplace (task);
          }
        );

      // terminate only the tasks assigned to the worker given as parameter
      for (auto const& task : started_tasks)
      {
        if (worker_tasks.count (task))
        {
          _scheduler->release_reservation (task);
          worker_tasks.erase (task);
        }
      }
    }

    BOOST_REQUIRE_EQUAL
      (count_assigned_jobs (get_current_assignment(), worker), 0);

    auto const all_tasks_before_stealing (all_assigned_tasks());

    _scheduler->assign_jobs_to_workers();

    CHECK_ALL_WORKERS_HAVE_AT_LEAST_ONE_TASK_ASSIGNED (test_workers);

    BOOST_REQUIRE_EQUAL (all_tasks_before_stealing, all_assigned_tasks());
  }
};

namespace
{
  const double computational_cost = 1.0;

  Requirements_and_preferences require (std::string name_1)
  {
    return { {we::type::Requirement (name_1)}
           , we::type::schedule_data()
           , null_transfer_cost
           , computational_cost
           , 0
           , {}
           };
  }

  Requirements_and_preferences no_requirements_and_preferences()
  {
    return { {}
           , we::type::schedule_data()
           , null_transfer_cost
           , computational_cost
           , 0
           , {}
           };
  }

  Requirements_and_preferences require
    ( std::string capability
    , Preferences const& preferences
    )
  {
    return { {we::type::Requirement (capability)}
           , we::type::schedule_data()
           , null_transfer_cost
           , computational_cost
           , 0
           , preferences
           };
  }

  void add_worker ( sdpa::daemon::WorkerManager& worker_manager
                  , sdpa::worker_id_t worker_id
                  , sdpa::Capabilities capabilities = {}
                  )
  {
    fhg::util::testing::random<std::string> const random_string;
    fhg::util::testing::random<unsigned short> const random_ushort;

    auto const hostname {random_string()};

    std::lock_guard<std::mutex> lock (worker_manager._mutex);
    worker_manager.add_worker
      ( worker_id
      , capabilities
      , fhg::util::testing::random<unsigned long>{}()
      , hostname
      , fhg::com::p2p::address_t
          { fhg::com::host_t {hostname}
          , fhg::com::port_t {random_ushort()}
          }
      );
  }

  void add_worker ( sdpa::daemon::WorkerManager& worker_manager
                  , sdpa::worker_id_t worker_id
                  , sdpa::Capabilities capabilities
                  , unsigned long allocated_shared_memory_size
                  , std::string const& hostname
                  , fhg::com::p2p::address_t const& address
                  )
  {
    std::lock_guard<std::mutex> lock (worker_manager._mutex);
    worker_manager.add_worker
      ( worker_id
      , capabilities
      , allocated_shared_memory_size
      , hostname
      , address
      );
  }

  void delete_worker
    ( sdpa::daemon::WorkerManager& worker_manager
    , sdpa::worker_id_t worker_id
    )
  {
    std::lock_guard<std::mutex> lock (worker_manager._mutex);
    worker_manager.delete_worker (worker_id);
  }
}

MULTIPLE_FIXTURES_TEST_CASE
  ( load_balancing
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  )
{
  add_worker (F::_worker_manager, "worker_0");
  add_worker (F::_worker_manager, "worker_1");

  const unsigned int n_jobs (100);
  for (unsigned int i (0); i < n_jobs; ++i)
  {
    F::add_and_enqueue_job (no_requirements_and_preferences());
  }

  F::_scheduler->assign_jobs_to_workers();
  auto const assignment (F::get_current_assignment());

  auto const assigned_to_worker_0
    (F::count_assigned_jobs (assignment, "worker_0"));
  auto const assigned_to_worker_1
    (F::count_assigned_jobs (assignment, "worker_1"));

  BOOST_REQUIRE (  (assigned_to_worker_0 <= assigned_to_worker_1 + 1L)
                && (assigned_to_worker_1 <= assigned_to_worker_0 + 1L)
                );
}

MULTIPLE_FIXTURES_TEST_CASE
  ( tesLBOneWorkerJoinsLater
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::GreedyScheduler>
  )
{
  add_worker (F::_worker_manager, "worker_0");

  F::add_and_enqueue_job (no_requirements_and_preferences());
  F::add_and_enqueue_job (no_requirements_and_preferences());

  {
    F::_scheduler->assign_jobs_to_workers();
    auto const assignment (F::get_current_assignment());

    BOOST_REQUIRE_EQUAL (F::count_assigned_jobs (assignment, "worker_0"), 2);
  }

  add_worker (F::_worker_manager, "worker_1");

  {
    F::_scheduler->assign_jobs_to_workers();
    auto const assignment (F::get_current_assignment());

    BOOST_REQUIRE_EQUAL (F::count_assigned_jobs (assignment, "worker_0"), 1);
    BOOST_REQUIRE_EQUAL (F::count_assigned_jobs (assignment, "worker_1"), 1);
  }
}

MULTIPLE_FIXTURES_TEST_CASE
  ( tesLBStopRestartWorker
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  )
{
  add_worker (F::_worker_manager, "worker_0");
  add_worker (F::_worker_manager, "worker_1");

  auto const job_0
    (F::add_and_enqueue_job (no_requirements_and_preferences()));
  auto const job_1
    (F::add_and_enqueue_job (no_requirements_and_preferences()));

  F::_scheduler->assign_jobs_to_workers();
  auto const assignment (F::get_current_assignment());

  BOOST_REQUIRE ( assignment.at (job_0) == set_workers_t ({"worker_0"})
               || assignment.at (job_0) == set_workers_t ({"worker_1"})
                );

  BOOST_REQUIRE ( assignment.at (job_1) == set_workers_t ({"worker_0"})
               || assignment.at (job_1) == set_workers_t ({"worker_1"})
                );

  BOOST_REQUIRE( assignment.at (job_0) !=  assignment.at (job_1));

  sdpa::job_id_t job_assigned_to_worker_0
    (assignment.at (job_0) == set_workers_t ({"worker_0"}) ? job_0 : job_1);

  F::_scheduler->release_reservation (job_assigned_to_worker_0);
  delete_worker (F::_worker_manager, "worker_0");
  F::_scheduler->submit_job (job_assigned_to_worker_0);

  add_worker (F::_worker_manager, "worker_0");

  F::_scheduler->assign_jobs_to_workers();
  auto const new_assignment (F::get_current_assignment());

  BOOST_REQUIRE_EQUAL ( new_assignment.at (job_assigned_to_worker_0)
                      , set_workers_t ({"worker_0"})
                      );
}

MULTIPLE_FIXTURES_TEST_CASE
  ( multiple_job_submissions_with_no_children_allowed
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::GreedyScheduler>
  )
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());
  add_worker (F::_worker_manager, worker_id);

  auto const job_id_0
    (F::add_and_enqueue_job (no_requirements_and_preferences()));

  {
    F::_scheduler->assign_jobs_to_workers();
    auto const assignment (F::get_current_assignment());

    BOOST_REQUIRE_EQUAL ( assignment.at (job_id_0)
                        , set_workers_t ({worker_id})
                        );
  }

  F::_scheduler->start_pending_jobs (SERVE_JOB_AND_CHECK_EXPECTED (job_id_0));

  auto const job_id_1
    (F::add_and_enqueue_job (no_requirements_and_preferences()));

  {
    F::_scheduler->assign_jobs_to_workers();
    auto const assignment (F::get_current_assignment());

    BOOST_REQUIRE_EQUAL ( assignment.at (job_id_1)
                        , set_workers_t ({worker_id})
                        );
  }

  BOOST_REQUIRE_NO_THROW
    (F::_scheduler->start_pending_jobs (EXPECT_NO_JOB_SERVED));

  F::_scheduler->release_reservation (job_id_0);

  F::_scheduler->start_pending_jobs (SERVE_JOB_AND_CHECK_EXPECTED (job_id_1));
}

MULTIPLE_FIXTURES_TEST_CASE
  ( multiple_worker_job_submissions_with_requirements_and_preferences_no_children_allowed
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::GreedyScheduler>
  )
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());
  add_worker ( F::_worker_manager
             , worker_id
             , { sdpa::Capability ("A")
               , sdpa::Capability ("B")
               }
             );

  auto const job_id_0 (F::add_and_enqueue_job (require ("A")));

  {
    F::_scheduler->assign_jobs_to_workers();
    auto const assignment (F::get_current_assignment());

    BOOST_REQUIRE_EQUAL ( assignment.at (job_id_0)
                        , set_workers_t ({worker_id})
                        );
  }

  F::_scheduler->start_pending_jobs (SERVE_JOB_AND_CHECK_EXPECTED (job_id_0));

  auto const job_id_1 (F::add_and_enqueue_job (require ("B")));

  {
    F::_scheduler->assign_jobs_to_workers();
    auto const assignment (F::get_current_assignment());

    BOOST_REQUIRE_EQUAL ( assignment.at (job_id_1)
                        , set_workers_t ({worker_id})
                        );
  }

  BOOST_REQUIRE_NO_THROW (F::_scheduler->start_pending_jobs (EXPECT_NO_JOB_SERVED));

  F::_scheduler->release_reservation (job_id_0);

  F::_scheduler->start_pending_jobs (SERVE_JOB_AND_CHECK_EXPECTED (job_id_1));
}

template <typename Scheduler>
struct serve_job_and_check_for_minimal_cost_assignement
{
  serve_job_and_check_for_minimal_cost_assignement()
    : _worker_ids
        ( generate_worker_names
            (fhg::util::testing::random<unsigned int>{} (20, 10))
        )
    , _map_transfer_costs (generate_costs (_worker_ids))
    , _transfer_cost
        ( [this](std::string const& worker) -> double
          {
            return _map_transfer_costs.at (worker);
          }
        )
    , _worker_manager()
    , _scheduler
       ( std::make_unique<Scheduler>
           ( [this] (sdpa::job_id_t const&)
             {
               return Requirements_and_preferences
                 ( {}
                 , we::type::schedule_data()
                 , _transfer_cost
                 , computational_cost
                 , 0
                 , {}
                 );
             }
           , _worker_manager
           )
       )
  {}

  std::vector<std::string> generate_worker_names (unsigned int n)
  {
    std::vector<std::string> worker_ids (n);
    std::generate ( worker_ids.begin()
                  , worker_ids.end()
                  , utils::random_peer_name
                  );
    return worker_ids;
  }

  std::map<sdpa::worker_id_t, double>
    generate_costs (std::vector<std::string> worker_ids)
  {
    std::normal_distribution<> dist (0., 1.);

    std::map<sdpa::worker_id_t, double> map_costs;
    for (sdpa::worker_id_t const& worker : worker_ids)
    {
      map_costs.emplace
        (worker, dist (fhg::util::testing::detail::GLOBAL_random_engine()));
    }

    return map_costs;
  }

  void serve_and_check_assignment
    ( sdpa::daemon::WorkerSet const& assigned_workers
    , sdpa::daemon::Implementation const&
    , sdpa::job_id_t const&
    )
  {
    sdpa::worker_id_t assigned_worker_with_max_cost
      ( *std::max_element
          ( assigned_workers.begin()
          , assigned_workers.end()
          , [this] ( sdpa::worker_id_t const& left
                   , sdpa::worker_id_t const& right
                   )
            { return _transfer_cost (left) < _transfer_cost (right); }
          )
      );

    for (sdpa::worker_id_t const& wid : _worker_ids)
    {
      if (!assigned_workers.count (wid))
      {
        // any not assigned worker has n associated a cost that is either greater or equal
        // to the maximum cost of the assigned workers
        BOOST_REQUIRE_GE
          (_transfer_cost (wid), _transfer_cost (assigned_worker_with_max_cost));
      }
    }
  }

  std::vector<std::string> _worker_ids;
  std::map<sdpa::worker_id_t, double> _map_transfer_costs;
  std::function<double (std::string const&)> _transfer_cost;
  sdpa::daemon::WorkerManager _worker_manager;
  std::unique_ptr<sdpa::daemon::Scheduler> _scheduler;
};

MULTIPLE_FIXTURES_TEST_CASE
  ( scheduling_with_data_locality_and_random_costs
  , F
  , serve_job_and_check_for_minimal_cost_assignement
      <sdpa::daemon::SingleAllocationScheduler>
  , serve_job_and_check_for_minimal_cost_assignement
      <sdpa::daemon::CoallocationScheduler>
  )
{
  fhg::util::testing::random<unsigned short> const random_ushort;

  for (sdpa::worker_id_t const& worker_id : F::_worker_ids)
  {
    add_worker
      ( F::_worker_manager
      , worker_id
      , {}
      , random_ulong()
      , worker_id
      , fhg::com::p2p::address_t
          { fhg::com::host_t {worker_id}
          , fhg::com::port_t {random_ushort()}
          }
      );
  }

  const sdpa::job_id_t job_id (fhg::util::testing::random_string());
  F::_scheduler->submit_job (job_id);

  F::_scheduler->assign_jobs_to_workers();
  F::_scheduler->start_pending_jobs
    ( [this, &job_id]
      ( sdpa::daemon::WorkerSet const& assigned_workers
      , sdpa::daemon::Implementation const& implementation
      , sdpa::job_id_t const& started_job
      , std::function<fhg::com::p2p::address_t (sdpa::worker_id_t const&)>
      )
      {
        BOOST_REQUIRE_EQUAL (job_id, started_job);

        F::serve_and_check_assignment
          ( assigned_workers
          , implementation
          , started_job
          );
      }
    );
}

template <typename Scheduler>
struct fixture_preassignment_and_load_balancing
{
  fixture_preassignment_and_load_balancing()
    : _worker_ids (_n_workers)
    , _n_hosts (_n_workers)
    , _host_ids (_n_hosts)
    , _dist (1.0, _n_hosts)
    , _rand_engine (fhg::util::testing::detail::GLOBAL_random_engine())
    , _computational_cost (_dist (_rand_engine))
    , _transfer_costs (_n_hosts)
    , _test_transfer_cost
        ( [this]
          (std::string const& host) -> double
          {
            std::vector<std::string>::const_iterator
              it (std::find (_host_ids.begin(), _host_ids.end(), host));
            if (it != _host_ids.end())
            {
              return  _transfer_costs[it - _host_ids.begin()];
            }
            else
            {
              throw std::runtime_error
                ("Unexpected host argument in transfer cost function");
            }
          }
       )
    , _worker_manager()
    , _scheduler
      ( std::make_unique<Scheduler>
          ( [this]
            (sdpa::job_id_t const&)
            {
              return Requirements_and_preferences
                ( {}
                , we::type::schedule_data()
                , _test_transfer_cost
                , _computational_cost
                , 0
                , {}
                );
            }
          , _worker_manager
          )
      )
    , _access_allocation_table
        (dynamic_cast<Scheduler*> (_scheduler.get())->strategy_TESTING_ONLY())
  {
    std::generate_n (_worker_ids.begin(), _n_workers, utils::random_peer_name);
    std::generate_n (_host_ids.begin(), _n_hosts, utils::random_peer_name);
    std::generate_n ( _transfer_costs.begin()
                    , _n_hosts
                    , [this]() { return _dist (_rand_engine); }
                    );

  }

  unsigned int _n_workers {20};
  std::vector<sdpa::worker_id_t> _worker_ids;
  unsigned int _n_hosts;
  std::vector<sdpa::worker_id_t> _host_ids;
  std::uniform_real_distribution<double> _dist;
  fhg::util::testing::detail::RandomNumberEngine& _rand_engine;
  double _computational_cost;
  std::vector<double> _transfer_costs;
  std::function<double (std::string const&)> _test_transfer_cost;
  sdpa::daemon::WorkerManager _worker_manager;
  std::unique_ptr<sdpa::daemon::Scheduler> _scheduler;
  sdpa::daemon::access_allocation_table_TESTING_ONLY _access_allocation_table;
};

MULTIPLE_FIXTURES_TEST_CASE
  ( scheduling_bunch_of_jobs_with_preassignment_and_load_balancing
  , F
  , fixture_preassignment_and_load_balancing
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_preassignment_and_load_balancing
      <sdpa::daemon::CoallocationScheduler>
  )
{
  const unsigned int n_jobs (1000);
  fhg::util::testing::random<unsigned short> const random_ushort;

  for (unsigned int i (0); i < F::_n_workers; ++i)
  {
    add_worker
      ( F::_worker_manager
      , F::_worker_ids[i]
      , {}
      , random_ulong()
      , F::_host_ids[i]
      , fhg::com::p2p::address_t
          { fhg::com::host_t { F::_host_ids[i]}
          , fhg::com::port_t {random_ushort()}
          }
      );
  }

  fhg::util::testing::unique_random<sdpa::job_id_t> job_ids;
  for (unsigned int i (0); i < n_jobs; ++i)
  {
    F::_scheduler->submit_job (job_ids());
  }

  F::_scheduler->assign_jobs_to_workers();
  auto const assignment
    (F::_access_allocation_table. get_current_assignment());

  std::vector<double> sum_costs_assigned_jobs (F::_n_workers, 0.0);
  const double max_job_cost
    ( *std::max_element (F::_transfer_costs.begin(), F::_transfer_costs.end())
    + F::_computational_cost
    );

  for ( std::set<sdpa::worker_id_t> const& job_assigned_workers
      : assignment | ::boost::adaptors::map_values
      )
  {
    for (unsigned int k (0); k < F::_n_workers; ++k)
    {
      sum_costs_assigned_jobs[k] += job_assigned_workers.count (F::_worker_ids[k])
                                  * (F::_transfer_costs[k] + F::_computational_cost);
    }
  }

  BOOST_REQUIRE_LE
    ( std::abs ( *std::max_element
                   ( sum_costs_assigned_jobs.begin()
                   , sum_costs_assigned_jobs.end()
                   )
               - *std::min_element
                   ( sum_costs_assigned_jobs.begin()
                   , sum_costs_assigned_jobs.end()
                   )
               )
    , max_job_cost
    );
}

MULTIPLE_FIXTURES_TEST_CASE
  ( no_assignment_if_not_enough_memory
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  )
{
  unsigned long avail_mem (random_ulong());
  if (avail_mem > 0) avail_mem--;

  fhg::util::testing::random<std::string> const random_string;
  fhg::util::testing::random<unsigned short> const random_ushort;
  auto const hostname {random_string()};

  add_worker
    ( F::_worker_manager
    , "worker_0"
    , {}
    , avail_mem
    , hostname
    , fhg::com::p2p::address_t
        { fhg::com::host_t {hostname}
        , fhg::com::port_t {random_ushort()}
        }
    );

  F::add_and_enqueue_job
    ( Requirements_and_preferences ( {}
                                   , we::type::schedule_data()
                                   , null_transfer_cost
                                   , computational_cost
                                   , avail_mem + 1
                                   , {}
                                   )
    );

  F::_scheduler->assign_jobs_to_workers();

  BOOST_REQUIRE (F::get_current_assignment().empty());
}

MULTIPLE_FIXTURES_TEST_CASE
  ( invariant_assignment_for_jobs_with_different_memory_requirements_and_preferences
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  )
{
  unsigned int size_0 (1000);
  unsigned int size_1 (2000);
  std::set<sdpa::worker_id_t> set_0 {"worker_0"};
  std::set<sdpa::worker_id_t> set_1 {"worker_1"};

  fhg::util::testing::random<std::string> const random_string;
  fhg::util::testing::random<unsigned short> const random_ushort;

  {
    auto const hostname {random_string()};
    add_worker
      ( F::_worker_manager
      , "worker_0"
      , {}
      , size_0
      , hostname
      , fhg::com::p2p::address_t
          { fhg::com::host_t {hostname}
          , fhg::com::port_t {random_ushort()}
          }
      );
  }

  {
    auto const hostname {random_string()};
    add_worker
      (  F::_worker_manager
      , "worker_1"
      , {}
      , size_1
      , hostname
      , fhg::com::p2p::address_t
          { fhg::com::host_t {hostname}
          , fhg::com::port_t {random_ushort()}
          }
      );
  }

  {
    auto const hostname {random_string()};
    add_worker
      ( F::_worker_manager
      , "worker_2"
      , {}
      , size_0 + size_1
      , hostname
      , fhg::com::p2p::address_t
          { fhg::com::host_t {hostname}
          , fhg::com::port_t {random_ushort()}
          }
      );
  }

  auto const job_id_0
    ( F::add_and_enqueue_job
        ( Requirements_and_preferences ( {}
                                       , we::type::schedule_data()
                                       , null_transfer_cost
                                       , computational_cost
                                       , size_0
                                       , {}
                                       )
      )
    );

  auto const job_id_1
    ( F::add_and_enqueue_job
        ( Requirements_and_preferences ( {}
                                       , we::type::schedule_data()
                                       , null_transfer_cost
                                       , computational_cost
                                       , size_1
                                       , {}
                                       )
        )
    );

  {
    F::_scheduler->assign_jobs_to_workers();
    auto const assignment (F::get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());
    BOOST_REQUIRE (assignment.count (job_id_0));
    BOOST_REQUIRE (assignment.count (job_id_1));

    BOOST_REQUIRE_EQUAL (assignment.at (job_id_0), set_0);
    BOOST_REQUIRE_EQUAL (assignment.at (job_id_1), set_1);
  }

  F::_scheduler->release_reservation (job_id_0);
  F::_scheduler->release_reservation (job_id_1);

  F::_scheduler->submit_job (job_id_1);
  F::_scheduler->submit_job (job_id_0);

  {
    F::_scheduler->assign_jobs_to_workers();
    auto const assignment (F::get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());
    BOOST_REQUIRE (assignment.count (job_id_0));
    BOOST_REQUIRE (assignment.count (job_id_1));

    BOOST_REQUIRE_EQUAL (assignment.at (job_id_0), set_0);
    BOOST_REQUIRE_EQUAL (assignment.at (job_id_1), set_1);
  }
}

MULTIPLE_FIXTURES_TEST_CASE
  ( assign_job_without_requirements_and_preferences_to_worker_with_least_capabilities
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  )
{
  std::string const name_worker_0 {utils::random_peer_name()};
  std::string const name_worker_1 {utils::random_peer_name()};
  std::string const name_capability {fhg::util::testing::random_string()};
  add_worker (F::_worker_manager, name_worker_0);
  add_worker ( F::_worker_manager
             , name_worker_1
             , {sdpa::Capability (name_capability)}
             );

  auto const job_id (F::add_and_enqueue_job (no_requirements_and_preferences()));

  F::_scheduler->assign_jobs_to_workers();
  auto const assignment (F::get_current_assignment());

  BOOST_REQUIRE_EQUAL (assignment.size(), 1);
  BOOST_REQUIRE (assignment.count (job_id));

  BOOST_REQUIRE_EQUAL
    (assignment.at (job_id), std::set<sdpa::worker_id_t> {name_worker_0});
}

MULTIPLE_FIXTURES_TEST_CASE
  ( assign_job_to_the_matching_worker_with_less_capabilities_when_same_costs
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  )
{
  std::set<sdpa::worker_id_t> set_0 {"worker_0"};

  add_worker ( F::_worker_manager
             , "worker_0"
             , {sdpa::Capability ("A")}
             );

  add_worker ( F::_worker_manager
             , "worker_1"
             , { sdpa::Capability ("A")
               , sdpa::Capability ("B")
               }
             );

  add_worker ( F::_worker_manager
             , "worker_2"
             , { sdpa::Capability ("A")
               , sdpa::Capability ("B")
               , sdpa::Capability ("C")
               }
             );

  auto const job_id (F::add_and_enqueue_job (require ("A")));

  F::_scheduler->assign_jobs_to_workers();
  auto const assignment (F::get_current_assignment());

  BOOST_REQUIRE (!assignment.empty());
  BOOST_REQUIRE (assignment.count (job_id));

  BOOST_REQUIRE_EQUAL (assignment.at (job_id), set_0);
}

MULTIPLE_FIXTURES_TEST_CASE
  ( assign_to_the_same_worker_if_the_total_cost_is_lower
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  )
{
  std::string const name_worker_0 {utils::random_peer_name()};
  std::string const name_worker_1 {utils::random_peer_name()};
  std::string const name_node_0 {utils::random_peer_name()};
  std::string const name_node_1 {utils::random_peer_name()};

  std::set<sdpa::worker_id_t> const expected_assignment {name_worker_1};

  fhg::util::testing::random<unsigned short> const random_ushort;

  add_worker
   ( F::_worker_manager
   , name_worker_0
   , {}
   , 199
   , name_node_0
   , fhg::com::p2p::address_t
       { fhg::com::host_t {name_node_0}
       , fhg::com::port_t {random_ushort()}
       }
   );

  add_worker
    ( F::_worker_manager
    , name_worker_1
    , {}
    , 200
    , name_node_1
    , fhg::com::p2p::address_t
        { fhg::com::host_t {name_node_1}
        , fhg::com::port_t {random_ushort()}
        }
    );

  std::function<double (std::string const&)> const
    test_transfer_cost ( [&name_node_0, &name_node_1](std::string const& host) -> double
                         {
                           if (host == name_node_0)
                             return 1000.0;
                           if (host == name_node_1)
                             return 1.0;
                           throw std::runtime_error ("Unexpected host argument in test transfer cost function");
                         }
                       );

  std::set<sdpa::job_id_t> jobs;

  for ( unsigned long k = 0
      ; k < fhg::util::testing::random<std::size_t>{} (100, 3)
      ; ++k
      )
  {
    jobs.emplace
      ( F::add_and_enqueue_job
          ( Requirements_and_preferences ( {}
                                         , we::type::schedule_data()
                                         , test_transfer_cost
                                         , 1.0
                                         , 100
                                         , {}
                                         )
          )
      );
  }

  F::_scheduler->assign_jobs_to_workers();
  auto const assignment (F::get_current_assignment());

  BOOST_REQUIRE (!assignment.empty());
  for (auto const& job_id : jobs)
  {
    BOOST_REQUIRE (assignment.count (job_id));
  }

  // No worker should be idle and in the case when new jobs are added, these
  // should be assigned to the worker that minimizes the total cost.
  // In this test case, they are assigned to worker_1, because the transfer
  // cost is much smaller for this, any time a new job is added,
  BOOST_REQUIRE_EQUAL (F::count_assigned_jobs (assignment, name_worker_0), 1);
  BOOST_REQUIRE_EQUAL
    (F::count_assigned_jobs (assignment, name_worker_1), assignment.size() - 1);
}

MULTIPLE_FIXTURES_TEST_CASE
  ( work_stealing
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::GreedyScheduler>
  )
{
  std::set<sdpa::worker_id_t> set_0 {"worker_0"};

  add_worker
    (F::_worker_manager, "worker_0", {sdpa::Capability ("A")});
  add_worker
    (F::_worker_manager, "worker_1", {sdpa::Capability ("A")});

  auto const job_0 (F::add_and_enqueue_job (require ("A")));
  auto const job_1 (F::add_and_enqueue_job (require ("A")));
  auto const job_2 (F::add_and_enqueue_job (require ("A")));

  {
    // all jobs are assigned to either worker_0 or worker_1
    F::_scheduler->assign_jobs_to_workers();
    auto const assignment (F::get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());
    BOOST_REQUIRE (assignment.count (job_0));
    BOOST_REQUIRE (assignment.count (job_1));
    BOOST_REQUIRE (assignment.count (job_2));
  }

  add_worker
    (F::_worker_manager, "worker_2", {sdpa::Capability ("A")});

  // worker_2 should steal a job either from worker_0 or worker_1
  F::_scheduler->assign_jobs_to_workers();

  {
    auto const assignment (F::get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());
    BOOST_REQUIRE (assignment.count (job_0));
    BOOST_REQUIRE (assignment.count (job_1));
    BOOST_REQUIRE (assignment.count (job_2));

    BOOST_REQUIRE (  assignment.at (job_0) == std::set<sdpa::worker_id_t>({"worker_2"})
                  || assignment.at (job_1) == std::set<sdpa::worker_id_t>({"worker_2"})
                  || assignment.at (job_2) == std::set<sdpa::worker_id_t>({"worker_2"})
                  );

    // all jobs should be assigned to distinct workers
    BOOST_REQUIRE (assignment.at (job_0) != assignment.at (job_1));
    BOOST_REQUIRE (assignment.at (job_0) != assignment.at (job_2));
    BOOST_REQUIRE (assignment.at (job_1) != assignment.at (job_2));
  }
}

MULTIPLE_FIXTURES_TEST_CASE
  ( stealing_from_worker_does_not_free_it
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::GreedyScheduler>
  )
{
  add_worker
    (F::_worker_manager, "worker_0", {sdpa::Capability ("A")});

  auto const job_0 (F::add_and_enqueue_job (require ("A")));
  auto const job_1 (F::add_and_enqueue_job (require ("A")));
  auto const job_2 (F::add_and_enqueue_job (require ("A")));

  {
    F::_scheduler->assign_jobs_to_workers();
    auto const assignment (F::get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());
    BOOST_REQUIRE (assignment.count (job_0));
    BOOST_REQUIRE (assignment.count (job_1));
    BOOST_REQUIRE (assignment.count (job_2));

    // the worker 0 is submitted one of the assigned jobs
    std::set<sdpa::job_id_t> const jobs_that_can_be_started {job_0, job_1, job_2};
    F::_scheduler->start_pending_jobs
      (SERVE_JOB_AND_CHECK_IS_ONE_OF (jobs_that_can_be_started));
  }

  add_worker
    (F::_worker_manager, "worker_1", {sdpa::Capability ("A")});

  F::_scheduler->assign_jobs_to_workers();

  {
    auto const assignment (F::get_current_assignment());
    BOOST_REQUIRE (!assignment.empty());

    // the worker 1 is assigned a job that was stolen from the worker 0
    BOOST_REQUIRE (  assignment.at (job_0) == std::set<sdpa::worker_id_t>({"worker_1"})
                  || assignment.at (job_1) == std::set<sdpa::worker_id_t>({"worker_1"})
                  || assignment.at (job_2) == std::set<sdpa::worker_id_t>({"worker_1"})
                  );

    // only the worker 1 should get a job started, as worker 0 is still reserved
    // (until the submitted job finishes)
    std::set<sdpa::job_id_t> const jobs_that_can_be_started {job_0, job_1, job_2};
    std::set<sdpa::job_id_t> jobs_started;
    F::_scheduler->start_pending_jobs
      ( [&jobs_that_can_be_started, &jobs_started]
        ( sdpa::daemon::WorkerSet const&
        , sdpa::daemon::Implementation const&
        , sdpa::job_id_t const& started_job
        , std::function<fhg::com::p2p::address_t (sdpa::worker_id_t const&)>
        )
        {
          BOOST_REQUIRE (jobs_that_can_be_started.count (started_job));
          jobs_started.emplace (started_job);
        }
      );

    BOOST_REQUIRE_EQUAL (jobs_started.size(), 1);
  }
}

template <typename Scheduler>
struct fixture_add_new_workers
{
  fixture_add_new_workers()
    : _worker_manager()
    , _scheduler
         ( std::make_unique<Scheduler>
             ( std::bind ( &fixture_add_new_workers::requirements_and_preferences
                         , this
                         , std::placeholders::_1
                         )
             , _worker_manager
             )
         )
    , _access_allocation_table
         (dynamic_cast<Scheduler*> (_scheduler.get())->strategy_TESTING_ONLY())
  {}

  sdpa::daemon::WorkerManager _worker_manager;
  std::unique_ptr<sdpa::daemon::Scheduler> _scheduler;
  sdpa::daemon::access_allocation_table_TESTING_ONLY _access_allocation_table;

  std::map<sdpa::job_id_t, std::set<sdpa::worker_id_t>> get_current_assignment() const
  {
    return _access_allocation_table.get_current_assignment();
  }

  sdpa::daemon::WorkerSet workers (sdpa::job_id_t const& job) const
  {
    return _access_allocation_table.workers (job);
  }

  sdpa::daemon::Implementation implementation (sdpa::job_id_t const& job) const
  {
    return _access_allocation_table.implementation (job);
  }

  fhg::util::testing::unique_random<sdpa::job_id_t> job_ids;
  sdpa::job_id_t add_and_enqueue_job
    (Requirements_and_preferences reqs_and_prefs)
  {
    auto const job_id (job_ids());
    _requirements_and_preferences.emplace (job_id, std::move (reqs_and_prefs));
    _scheduler->submit_job (job_id);
    return job_id;
  }

  Requirements_and_preferences requirements_and_preferences (sdpa::job_id_t id)
  {
    return _requirements_and_preferences.find (id)->second;
  }

  std::map<sdpa::job_id_t, Requirements_and_preferences>
    _requirements_and_preferences;

  std::vector<sdpa::worker_id_t> add_new_workers
    ( std::unordered_set<std::string> const& cpbnames
    , unsigned int n
    )
  {
    std::vector<sdpa::worker_id_t> new_workers (n);
    static unsigned int j (0);
    std::generate_n ( new_workers.begin()
                    , n
                    , [] {return "worker_" + std::to_string (j++);}
                    );

    for (sdpa::worker_id_t const& worker : new_workers)
    {
      sdpa::Capabilities cpbset;
      for (std::string const& capability_name : cpbnames)
      {
        cpbset.emplace (capability_name);
      }

      add_worker (_worker_manager, worker, cpbset);
      request_scheduling();
    }

    return new_workers;
  }

  void add_new_jobs
    ( std::vector<sdpa::job_id_t>& a
    , ::boost::optional<std::string> reqname
    , unsigned int n
    )
  {
    for (unsigned int i (0); i < n; ++i)
    {
      a.emplace_back
        ( add_and_enqueue_job
            (reqname ? require (*reqname) : no_requirements_and_preferences())
        );
      request_scheduling();
    }
  }

  void request_scheduling()
  {
    _scheduler->assign_jobs_to_workers();
  }

  std::set<sdpa::worker_id_t> get_workers_with_assigned_jobs
    (std::vector<sdpa::job_id_t> const& jobs)
  {
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());

    for (sdpa::job_id_t job : jobs)
    {
      BOOST_REQUIRE (assignment.count (job));
    }

    std::set<sdpa::worker_id_t> assigned_workers;
    for ( std::set<sdpa::worker_id_t> const& s
        : assignment | ::boost::adaptors::map_values
        )
    {
      std::set_union ( assigned_workers.begin()
                     , assigned_workers.end()
                     , s.begin()
                     , s.end()
                     , std::inserter (assigned_workers, assigned_workers.begin())
                     );
    }

    return assigned_workers;
  }

  std::set<sdpa::job_id_t> get_jobs_assigned_to_worker
    ( sdpa::worker_id_t const& worker
    , std::map<sdpa::job_id_t, std::set<sdpa::worker_id_t>>const& assignment
    )
  {
    BOOST_REQUIRE (!assignment.empty());

    std::set<sdpa::job_id_t> assigned_jobs;
    for (auto const& job_and_workers : assignment)
    {
      if (job_and_workers.second.count (worker))
      {
        assigned_jobs.emplace (job_and_workers.first);
      }
    }

    return assigned_jobs;
  }

  unsigned int n_jobs_assigned_to_worker
    ( sdpa::worker_id_t const& worker
    , std::map<sdpa::job_id_t, std::set<sdpa::worker_id_t>> assignment
    )
  {
    return get_jobs_assigned_to_worker (worker, assignment).size();
  }

  void check_work_stealing
    ( std::vector<sdpa::worker_id_t> const& initial_workers
    , std::vector<sdpa::worker_id_t> const& new_workers
    , std::map<sdpa::job_id_t, std::set<sdpa::worker_id_t>> old_assignment
    , unsigned int n_total_jobs
    )
  {
    unsigned int n_stolen_jobs (0);
    unsigned int n_jobs_initial_workers (0);
    auto const new_assignment (get_current_assignment());

    for (sdpa::worker_id_t worker : new_workers)
    {
      BOOST_REQUIRE_EQUAL
        (n_jobs_assigned_to_worker (worker, new_assignment), 1);
    }

    for (sdpa::worker_id_t worker : initial_workers)
    {
      const unsigned int n_new_jobs
        (n_jobs_assigned_to_worker (worker, new_assignment));
      BOOST_REQUIRE_GE (n_new_jobs, 1);

      const unsigned int n_jobs_diff
        ( n_jobs_assigned_to_worker (worker, old_assignment)
        - n_new_jobs
        );

      n_stolen_jobs += n_jobs_diff;
      n_jobs_initial_workers += n_new_jobs;
    }

    // Each of the new workers has stolen exactly one job
    BOOST_REQUIRE_EQUAL (n_stolen_jobs, new_workers.size());

    // the total number of jobs is conserved
    BOOST_REQUIRE_EQUAL (n_jobs_initial_workers + n_stolen_jobs, n_total_jobs);
  }

  void require_worker_and_implementation
    ( sdpa::job_id_t const& job
    , std::set<sdpa::worker_id_t>& expected_workers
    , sdpa::daemon::Implementation const& impl
    )
  {
    auto const assignment (get_current_assignment());
    BOOST_REQUIRE (assignment.count (job));

    auto const assigned_workers (workers (job));

    BOOST_REQUIRE_EQUAL (assigned_workers.size(), 1);
    BOOST_REQUIRE (implementation (job));
    BOOST_REQUIRE_EQUAL (implementation (job), impl);

    BOOST_REQUIRE
      (expected_workers.count (*assigned_workers.begin()));

    expected_workers.erase (*assigned_workers.begin());
  }
};

MULTIPLE_FIXTURES_TEST_CASE
  ( add_new_workers_and_steal_work
  , F
  , fixture_add_new_workers <sdpa::daemon::SingleAllocationScheduler>
  , fixture_add_new_workers <sdpa::daemon::CoallocationScheduler>
  , fixture_add_new_workers <sdpa::daemon::GreedyScheduler>
  )
{
  const unsigned int n (20);
  const unsigned int k (10);
  const unsigned int n_capabilities (3);

  fhg::util::testing::unique_random<std::string> gen_capabilities;
  std::unordered_set<std::string> capabilities;
  for (unsigned int i (0); i<n_capabilities; i++)
  {
    capabilities.emplace (gen_capabilities());
  }

  const std::vector<sdpa::worker_id_t> initial_workers
    (F::add_new_workers (capabilities, n));

  std::vector<sdpa::job_id_t> jobs;
  F::add_new_jobs (jobs, ::boost::none, 2*n);

  BOOST_REQUIRE_EQUAL (F::get_workers_with_assigned_jobs (jobs).size(), n);

  F::add_new_jobs (jobs, ::boost::none, 2*k);

  auto const old_assignment (F::get_current_assignment());

  const std::vector<sdpa::worker_id_t> new_workers
    (F::add_new_workers (capabilities, k));

  BOOST_REQUIRE_EQUAL (F::get_workers_with_assigned_jobs (jobs).size(), n + k);

  F::check_work_stealing (initial_workers, new_workers, old_assignment, jobs.size());
}

MULTIPLE_FIXTURES_TEST_CASE
  ( add_new_workers_with_capabilities_and_steal_work_repeatedly
  , F
  , fixture_add_new_workers <sdpa::daemon::SingleAllocationScheduler>
  , fixture_add_new_workers <sdpa::daemon::CoallocationScheduler>
  , fixture_add_new_workers <sdpa::daemon::GreedyScheduler>
  )
{
  std::vector<sdpa::worker_id_t> workers;
  std::vector<sdpa::job_id_t> jobs;

  const unsigned int n_load_jobs (100);
  const unsigned int n_calc_jobs (2000);
  const unsigned int n_reduce_jobs (100);

  const unsigned int n_load_workers (n_load_jobs);
  const unsigned int n_calc_workers (500);
  const unsigned int n_reduce_workers (n_reduce_jobs);

  F::add_new_workers ({"LOAD"}, n_load_workers);
  F::add_new_workers ({"REDUCE"}, n_reduce_workers);
  std::vector<sdpa::worker_id_t> initial_calc_workers
    (F::add_new_workers ({"CALC"}, n_calc_workers));

  F::add_new_jobs (jobs, std::string ("LOAD"), n_load_jobs);
  F::add_new_jobs (jobs, std::string ("CALC"), n_calc_jobs);
  F::add_new_jobs (jobs, std::string ("REDUCE"), n_reduce_jobs);

  BOOST_REQUIRE_EQUAL ( F::get_workers_with_assigned_jobs (jobs).size()
                      , n_load_workers
                      + n_calc_workers
                      + n_reduce_workers
                      );

  {
    // add a new LOAD worker
    const std::vector<sdpa::worker_id_t> new_load_workers
      (F::add_new_workers ({"LOAD"}, 1));

    // this last added worker should get nothing as there is no job new generated,
    // all the other jobs were already assigned and there is nothing to
    // steal as all the LOAD workers have exactly one job assigned
    BOOST_REQUIRE_EQUAL
      (F::n_jobs_assigned_to_worker (new_load_workers.front(), F::get_current_assignment()), 0);
  }

  {
    // add a new REDUCE worker
    const std::vector<sdpa::worker_id_t> new_reduce_workers
      (F::add_new_workers ({"REDUCE"}, 1));

    // this last added worker should get nothing as there is no job new generated,
    // all the other jobs were already assigned and there is nothing to
    // steal as all the REDUCE workers have exactly one job assigned
    BOOST_REQUIRE_EQUAL
      (F::n_jobs_assigned_to_worker (new_reduce_workers.front(), F::get_current_assignment()), 0);
   }

  {
    auto const old_assignment (F::get_current_assignment());

    // add new n_calc_workers CALC workers
    const std::vector<sdpa::worker_id_t> new_calc_workers
      (F::add_new_workers ({"CALC"}, n_calc_workers));

    BOOST_REQUIRE_EQUAL (new_calc_workers.size(), n_calc_workers);

    auto const assignment (F::get_current_assignment());

    // all CALC workers should get a job stolen from one of the  n_calc_workers
    // workers previously added (no new job was generated)
    F::check_work_stealing ( initial_calc_workers
                           , new_calc_workers
                           , old_assignment
                           , n_calc_jobs
                           );

    initial_calc_workers.insert ( initial_calc_workers.end()
                                , new_calc_workers.begin()
                                , new_calc_workers.end()
                                );
  }

  {
    auto const old_assignment (F::get_current_assignment());

    // add new n_calc_workers CALC workers
    const std::vector<sdpa::worker_id_t> new_calc_workers
      (F::add_new_workers ({"CALC"}, n_calc_workers));

    BOOST_REQUIRE_EQUAL (new_calc_workers.size(), n_calc_workers);

    auto const assignment (F::get_current_assignment());

    // all CALC workers should get a job stolen from one of the  n_calc_workers
    // workers previously added (no new job was generated)
    F::check_work_stealing ( initial_calc_workers
                           , new_calc_workers
                           , old_assignment
                           , n_calc_jobs
                           );
  }
}

MULTIPLE_FIXTURES_TEST_CASE
  ( steal_work_after_finishing_assigned_job
  , F
  , fixture_add_new_workers <sdpa::daemon::SingleAllocationScheduler>
  , fixture_add_new_workers <sdpa::daemon::CoallocationScheduler>
  , fixture_add_new_workers <sdpa::daemon::GreedyScheduler>
  )
{
  const unsigned int n_workers (2);
  constexpr unsigned int n_jobs (n_workers + 1);
  const unsigned int n_capabilities (3);

  fhg::util::testing::unique_random<std::string> gen_capabilities;
  std::unordered_set<std::string> capabilities;
  for (unsigned int i (0); i<n_capabilities; i++)
  {
    capabilities.emplace (gen_capabilities());
  }

  const std::vector<sdpa::worker_id_t> workers
    (F::add_new_workers (capabilities, n_workers));

  std::vector<sdpa::job_id_t> jobs;
  F::add_new_jobs (jobs, ::boost::none, n_jobs);

  BOOST_REQUIRE_EQUAL (F::get_workers_with_assigned_jobs (jobs).size(), n_workers);

  auto const old_assignment (F::get_current_assignment());

  sdpa::worker_id_t worker_with_2_jobs;
  sdpa::worker_id_t worker_with_1_job;

  for (sdpa::worker_id_t const& worker : workers)
  {
    const unsigned int n_assigned_jobs
      (F::n_jobs_assigned_to_worker (worker, old_assignment));
    BOOST_REQUIRE (n_assigned_jobs == 1 || n_assigned_jobs == 2);

    if (n_assigned_jobs == 2)
    {
      worker_with_2_jobs = worker;
    }
    else
    {
      worker_with_1_job = worker;
    }
  }

  BOOST_REQUIRE (!worker_with_2_jobs.empty());
  BOOST_REQUIRE (!worker_with_1_job.empty());

  // the worker with 1 job finishes his job
  const std::set<sdpa::job_id_t> worker_jobs
    (F::get_jobs_assigned_to_worker (worker_with_1_job, old_assignment));
  BOOST_REQUIRE_EQUAL (worker_jobs.size(), 1);

  F::_scheduler->start_pending_jobs
    ( [] ( sdpa::daemon::WorkerSet const&
         , sdpa::daemon::Implementation const&
         , sdpa::job_id_t const&
         , std::function<fhg::com::p2p::address_t (sdpa::worker_id_t const&)>
         )
      {}
    );

  F::_scheduler->release_reservation (*worker_jobs.begin());

  F::request_scheduling();

  auto const assignment (F::get_current_assignment());

  // the worker which just finished the job should steal from the worker with 2 jobs
  // in the end, all workers should have exactly one job assigned
  BOOST_REQUIRE_EQUAL (F::n_jobs_assigned_to_worker (worker_with_2_jobs, assignment), 1);
  BOOST_REQUIRE_EQUAL (F::n_jobs_assigned_to_worker (worker_with_1_job, assignment), 1);
}

MULTIPLE_FIXTURES_TEST_CASE
  ( cannot_steal_from_a_different_equivalence_class
  , F
  , fixture_add_new_workers <sdpa::daemon::SingleAllocationScheduler>
  , fixture_add_new_workers <sdpa::daemon::CoallocationScheduler>
  )
{
  const unsigned int n_workers (2);
  constexpr unsigned int n_jobs (n_workers + 1);

  const sdpa::worker_id_t worker_0
    (F::add_new_workers ({"A", "B"}, 1).at (0));

  const sdpa::worker_id_t worker_1
    (F::add_new_workers ({"A", "C"}, 1).at (0));

  std::vector<sdpa::job_id_t> jobs;
  F::add_new_jobs (jobs, std::string ("A"), n_jobs);

  BOOST_REQUIRE_EQUAL (F::get_workers_with_assigned_jobs (jobs).size(), n_workers);

  auto const old_assignment (F::get_current_assignment());

  const unsigned int n_assigned_jobs_worker_0
    (F::n_jobs_assigned_to_worker (worker_0, old_assignment));
  BOOST_REQUIRE (n_assigned_jobs_worker_0 == 1 || n_assigned_jobs_worker_0 == 2);

  const unsigned int n_assigned_jobs_worker_1
    (F::n_jobs_assigned_to_worker (worker_1, old_assignment));
  BOOST_REQUIRE (n_assigned_jobs_worker_1 == 1 || n_assigned_jobs_worker_1 == 2);

  const sdpa::worker_id_t worker_with_1_job
    {(n_assigned_jobs_worker_0 == 1) ? worker_0 : worker_1};
  const sdpa::worker_id_t worker_with_2_jobs
    {(n_assigned_jobs_worker_0 == 2) ? worker_0 : worker_1};

  const std::set<sdpa::job_id_t> worker_jobs
    (F::get_jobs_assigned_to_worker (worker_with_1_job, old_assignment));
  BOOST_REQUIRE_EQUAL (worker_jobs.size(), 1);

  F::_scheduler->start_pending_jobs
    ( [] ( sdpa::daemon::WorkerSet const&
         , sdpa::daemon::Implementation
         , sdpa::job_id_t const&
         , std::function<fhg::com::p2p::address_t (sdpa::worker_id_t const&)>
         )
      {}
    );

  // the worker with 1 job finishes the assigned job
  BOOST_REQUIRE (!worker_jobs.empty());
  F::_scheduler->release_reservation (*worker_jobs.begin());

  F::request_scheduling();

  auto const assignment (F::get_current_assignment());

  // the worker with 1 job and which just finished the job should NOT steal from
  // the worker with 2 jobs, as they are in different equivalence classes
  BOOST_REQUIRE_EQUAL (F::n_jobs_assigned_to_worker (worker_with_2_jobs, assignment), 2);
  BOOST_REQUIRE_EQUAL (F::n_jobs_assigned_to_worker (worker_with_1_job, assignment), 0);
}

MULTIPLE_FIXTURES_TEST_CASE
  ( the_worker_that_is_longer_idle_gets_first_the_job
  , F
  , fixture_add_new_workers <sdpa::daemon::SingleAllocationScheduler>
  , fixture_add_new_workers <sdpa::daemon::CoallocationScheduler>
  , fixture_add_new_workers <sdpa::daemon::GreedyScheduler>
  )
{
  const unsigned int n_workers (3);
  constexpr unsigned int n_jobs (n_workers + 1);

  const std::vector<sdpa::worker_id_t> workers
    (F::add_new_workers ({"A"}, 3));

  std::vector<sdpa::job_id_t> jobs;
  F::add_new_jobs (jobs, std::string ("A"), n_jobs);

  BOOST_REQUIRE_EQUAL (F::get_workers_with_assigned_jobs (jobs).size(), n_workers);

  sdpa::worker_id_t worker_with_2_jobs;
  auto const assignment (F::get_current_assignment());

  // only one worker should have 2 jobs assigned, the others just 1
  for (sdpa::worker_id_t const& worker : workers)
  {
    const unsigned int n_worker_jobs
      (F::n_jobs_assigned_to_worker (worker, assignment));
    BOOST_REQUIRE (n_worker_jobs == 1 || n_worker_jobs == 2);
    if (n_worker_jobs == 2)
    {
      // remember the worker with 2 jobs
      worker_with_2_jobs = worker;
    }
  }

  // take the workers with 1 job in reverse order
  std::vector<sdpa::worker_id_t> workers_with_1_job;
  std::copy_if ( workers.rbegin()
               , workers.rend()
               , back_inserter (workers_with_1_job)
               , [&worker_with_2_jobs] (sdpa::worker_id_t const& worker)
                 {
                   return worker != worker_with_2_jobs;
                 }
               );

  BOOST_REQUIRE_EQUAL (workers_with_1_job.size(), 2);

  // The workers having 1 job finish their job successively, after some delay.
  // The finishing order is inverse to the starting order.
  const std::chrono::milliseconds delay(20);
  for (sdpa::worker_id_t const& worker : workers_with_1_job)
  {
    const std::set<sdpa::job_id_t> worker_jobs
      (F::get_jobs_assigned_to_worker (worker, assignment));
    BOOST_REQUIRE_EQUAL (worker_jobs.size(), 1);

    F::_scheduler->start_pending_jobs
      ( [] ( sdpa::daemon::WorkerSet const&
           , sdpa::daemon::Implementation const&
           , sdpa::job_id_t const&
           , std::function<fhg::com::p2p::address_t (sdpa::worker_id_t const&)>
           )
        {}
      );

    F::_scheduler->release_reservation (*worker_jobs.begin());

    std::this_thread::sleep_for (delay);
  }

  {
    auto const assignment_ (F::get_current_assignment());

    // One worker has 2 pending jobs, 2 workers are idle, no jobs to assign are left
    BOOST_REQUIRE_EQUAL (F::n_jobs_assigned_to_worker (worker_with_2_jobs, assignment_), 2);
    BOOST_REQUIRE_EQUAL (F::n_jobs_assigned_to_worker (*workers_with_1_job.begin(), assignment_), 0);
    BOOST_REQUIRE_EQUAL (F::n_jobs_assigned_to_worker (*std::next (workers_with_1_job.begin()), assignment_), 0);
  }

  F::request_scheduling();

  {
    auto const assignment_ (F::get_current_assignment());

    // One of the idle workers should steal the only job to steal from
    // the worker with 2 pending jobs.
    BOOST_REQUIRE_EQUAL (F::n_jobs_assigned_to_worker (worker_with_2_jobs, assignment_), 1);
    BOOST_REQUIRE_EQUAL
      ( F::n_jobs_assigned_to_worker (*workers_with_1_job.begin(), assignment_)
      + F::n_jobs_assigned_to_worker (*std::next (workers_with_1_job.begin()), assignment_)
      , 1
      );
  }
}

MULTIPLE_FIXTURES_TEST_CASE
  ( assign_jobs_respecting_preferences
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  )
{
  fhg::util::testing::unique_random<std::string> capability_pool;

  std::string const common_capability (capability_pool());

  Preferences const preferences
    { capability_pool()
    , capability_pool()
    , capability_pool()
    };

  sdpa::worker_id_t const worker_0 (utils::random_peer_name());
  auto const first_pref (preferences.begin());

  add_worker ( F::_worker_manager
             , worker_0
             , { sdpa::Capability (common_capability)
               , sdpa::Capability (*first_pref)
               }
             );

  sdpa::worker_id_t const worker_1 (utils::random_peer_name());
  add_worker ( F::_worker_manager
             , worker_1
             , { sdpa::Capability (common_capability)
               , sdpa::Capability (*std::next (first_pref, 1))
               }
             );

  sdpa::worker_id_t const worker_2 (utils::random_peer_name());
  add_worker ( F::_worker_manager
             , worker_2
             , { sdpa::Capability (common_capability)
               , sdpa::Capability (*std::next (first_pref, 2))
               }
             );

  auto const job0
    (F::add_and_enqueue_job (require (common_capability, preferences)));
  F::_scheduler->assign_jobs_to_workers();
  F::require_worker_and_implementation (job0, worker_0, *first_pref);

  auto const job1
    (F::add_and_enqueue_job (require (common_capability, preferences)));
  F::_scheduler->assign_jobs_to_workers();
  F::require_worker_and_implementation (job1, worker_1, *std::next (first_pref, 1));

  auto const job2
    (F::add_and_enqueue_job (require (common_capability, preferences)));
  F::_scheduler->assign_jobs_to_workers();
  F::require_worker_and_implementation (job2, worker_2, *std::next (first_pref, 2));
}

MULTIPLE_FIXTURES_TEST_CASE
  ( check_worker_is_served_the_corresponding_implementation
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  )
{
  fhg::util::testing::unique_random<std::string> capability_pool;

  std::string const capability (capability_pool());

  Preferences const preferences
    { capability_pool()
    , capability_pool()
    , capability_pool()
    };

  auto const first_pref (preferences.begin());

  sdpa::worker_id_t const worker
    (fhg::util::testing::random_identifier_without_leading_underscore());

  auto const preference_id
    (fhg::util::testing::random<std::size_t>{} (preferences.size() - 1));
  auto const preference (*fhg::util::next (first_pref, preference_id));

  add_worker ( F::_worker_manager
             , worker
             , { sdpa::Capability (capability)
               , sdpa::Capability (preference)
               }
             );

  auto const job (F::add_and_enqueue_job (require (capability, preferences)));

  F::_scheduler->assign_jobs_to_workers();

  F::require_worker_and_implementation (job, worker, preference);

  F::_scheduler->start_pending_jobs
    ( [this]
      ( sdpa::daemon::WorkerSet const& assigned_workers
      , sdpa::daemon::Implementation const& implementation
      , sdpa::job_id_t const& job_
      , std::function<fhg::com::p2p::address_t (sdpa::worker_id_t const&)>
      )
      {
        BOOST_REQUIRE_EQUAL
          (F::_requirements_and_preferences.at (job_).numWorkers(), 1);

        BOOST_REQUIRE_EQUAL (this->workers (job_), assigned_workers);
        BOOST_REQUIRE_EQUAL (this->implementation (job_), implementation);
      }
    );
}

MULTIPLE_FIXTURES_TEST_CASE
  ( tasks_preferring_cpus_are_assigned_cpu_workers_first
  , F
  , fixture_add_new_workers <sdpa::daemon::SingleAllocationScheduler>
  , fixture_add_new_workers <sdpa::daemon::CoallocationScheduler>
  )
{
  const std::string CPU ("CPU");
  const std::string GPU ("GPU");

  fhg::util::testing::unique_random<std::string> capability_pool;

  std::string const common_capability (capability_pool());

  auto const num_cpu_workers
    (fhg::util::testing::random<unsigned int>{} (20, 10));

  std::vector<sdpa::worker_id_t> const cpu_workers
    (F::add_new_workers ( {common_capability, CPU}
                        , num_cpu_workers
                        )
    );

  std::set<sdpa::worker_id_t> expected_cpu_workers
    (cpu_workers.begin(), cpu_workers.end());

  auto const num_cpu_gpu_workers
    (fhg::util::testing::random<unsigned int>{} (20, 10));

  std::vector<sdpa::worker_id_t> const cpu_gpu_workers
    (F::add_new_workers ( {common_capability, CPU, GPU}
                        , num_cpu_gpu_workers
                        )
    );

  std::set<sdpa::worker_id_t> expected_cpu_gpu_workers
    (cpu_gpu_workers.begin(), cpu_gpu_workers.end());

  std::vector<std::string> targets (num_cpu_workers, CPU);
  targets.insert (targets.end(), num_cpu_gpu_workers, GPU);

  BOOST_REQUIRE_EQUAL (targets.size(), num_cpu_workers + num_cpu_gpu_workers);

  std::shuffle ( targets.begin()
               , targets.end()
               , fhg::util::testing::detail::GLOBAL_random_engine()
               );

  for (auto const& target : targets)
  {
    auto const job
      (F::add_and_enqueue_job (require (common_capability, {target})));

    F::request_scheduling();

    F::require_worker_and_implementation
      ( job
      , target == CPU ? expected_cpu_workers : expected_cpu_gpu_workers
      , target
      );
  }

  BOOST_REQUIRE (expected_cpu_workers.empty());
  BOOST_REQUIRE (expected_cpu_gpu_workers.empty());
}

MULTIPLE_FIXTURES_TEST_CASE
  ( random_workers_are_assigned_valid_implementations
  , F
  , fixture_add_new_workers <sdpa::daemon::SingleAllocationScheduler>
  , fixture_add_new_workers <sdpa::daemon::CoallocationScheduler>
  )
{
  fhg::util::testing::unique_random<std::string> capability_pool;

  std::string const common_capability (capability_pool());

  Preferences preferences;

  unsigned int total_num_workers (0);
  auto const num_preferences
    (fhg::util::testing::random<unsigned int>{} (12, 3));

  std::map<std::string, std::set<sdpa::worker_id_t>> workers_by_preference;

  for (unsigned int i (0); i < num_preferences; ++i)
  {
    std::string const preference (capability_pool());
    preferences.emplace_back (preference);

    auto const num_workers
      (fhg::util::testing::random<unsigned int>{} (200, 100));
    total_num_workers += num_workers;

    std::vector<sdpa::worker_id_t> const workers
      (F::add_new_workers ( {common_capability, preference}
                          , num_workers
                          )
      );

    workers_by_preference.emplace
      (preference, std::set<sdpa::worker_id_t> (workers.begin(), workers.end()));
  }

  auto const num_tasks
    ( fhg::util::testing::random<unsigned int>{}
        (3 * total_num_workers - 1, 2 * total_num_workers)
    );

  for (unsigned int i {0}; i < num_tasks; i++)
  {
    auto const task
      (F::add_and_enqueue_job (require (common_capability, preferences)));

    F::request_scheduling();

    auto const assignment (F::get_current_assignment());
    BOOST_REQUIRE (assignment.count (task));

    auto const assigned_implementation (*F::implementation (task));
    BOOST_REQUIRE (F::implementation (task));
    BOOST_REQUIRE
      ( std::find (preferences.begin(), preferences.end(), assigned_implementation)
      != preferences.end()
      );

    auto const assigned_worker (*F::workers (task).begin());

    BOOST_REQUIRE
      (workers_by_preference.at (assigned_implementation).count (assigned_worker));
  }
}

MULTIPLE_FIXTURES_TEST_CASE
  ( tasks_without_preferences_are_assigned_to_workers_with_least_capabilities
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::GreedyScheduler>
  )
{
  fhg::util::testing::unique_random<std::string> capability_pool;

  std::string const preference (capability_pool());

  sdpa::worker_id_t const worker_0 (utils::random_peer_name());
  add_worker ( F::_worker_manager
             , worker_0
             , {sdpa::Capability (preference)}
             );

  sdpa::worker_id_t const worker_1 (utils::random_peer_name());
  add_worker ( F::_worker_manager
             , worker_1
             , {}
             );

  auto const job_0
    ( F::add_and_enqueue_job
        ( Requirements_and_preferences
            ( {}
            , {}
            , null_transfer_cost
            , computational_cost
            , 0
            , {}
            )
        )
    );

  F::_scheduler->assign_jobs_to_workers();

  F::require_worker_and_implementation (job_0, worker_1, ::boost::none);

  auto const job_1
    ( F::add_and_enqueue_job
        ( Requirements_and_preferences
            ( {}
            , {}
            , null_transfer_cost
            , computational_cost
            , 0
            , {preference}
            )
        )
    );

  F::_scheduler->assign_jobs_to_workers();

  F::require_worker_and_implementation (job_1, worker_0, preference);

  auto const job_2
    ( F::add_and_enqueue_job
        ( Requirements_and_preferences
            ( {}
            , {}
            , null_transfer_cost
            , computational_cost
            , 0
            , {preference}
            )
        )
    );

  F::_scheduler->assign_jobs_to_workers();

  F::require_worker_and_implementation (job_2, worker_0, preference);

  auto const job_3
   ( F::add_and_enqueue_job
       ( Requirements_and_preferences
           ( {}
           , {}
           , null_transfer_cost
           , computational_cost
           , 0
           , {}
           )
       )
   );

   F::_scheduler->assign_jobs_to_workers();

   F::require_worker_and_implementation (job_3, worker_1, ::boost::none);
}

MULTIPLE_FIXTURES_TEST_CASE
  ( no_assignment_to_workers_with_no_capability_among_preferences_is_allowed
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  )
{
  fhg::util::testing::unique_random<std::string> capability_pool;

  sdpa::worker_id_t const worker (utils::random_peer_name());
  add_worker
    (F::_worker_manager, worker, {sdpa::Capability (capability_pool())});

  auto const job
    ( F::add_and_enqueue_job
        ( Requirements_and_preferences
            ( {}
            , {}
            , null_transfer_cost
            , computational_cost
            , 0
            , {capability_pool(), capability_pool(), capability_pool()}
            )
        )
    );

  F::_scheduler->assign_jobs_to_workers();

  auto const assignment (F::get_current_assignment());
  BOOST_REQUIRE_EQUAL (assignment.count (job), 0);
}

MULTIPLE_FIXTURES_TEST_CASE
  ( stealing_tasks_from_the_same_class
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::GreedyScheduler>
  )
{
  fhg::util::testing::unique_random<std::string> capability_pool;

  std::string const common_capability (capability_pool());

  auto const num_workers (fhg::util::testing::random<std::size_t>{} (100, 10));
  auto const num_tasks
    ( fhg::util::testing::random<std::size_t>{} (10, 2)
    * num_workers
    );

  std::vector<sdpa::worker_id_t> test_workers;

  for (unsigned int k (0); k < num_workers; ++k)
  {
    sdpa::worker_id_t const worker (utils::random_peer_name());
    test_workers.emplace_back (worker);
    add_worker ( F::_worker_manager
               , worker
               , {sdpa::Capability (common_capability)}
               );
  }

  for (unsigned int i {0}; i < num_tasks; ++i)
  {
    F::add_and_enqueue_job (require (common_capability));
  }

  F::_scheduler->assign_jobs_to_workers();

  F::finish_tasks_and_steal_work_when_idle_workers_exist
    (num_tasks, test_workers);
}

MULTIPLE_FIXTURES_TEST_CASE
  ( stealing_tasks_with_preferences
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  )
{
  fhg::util::testing::unique_random<std::string> capability_pool;

  std::string const common_capability (capability_pool());

  unsigned int const num_preferences (10);
  Preferences preferences;
  std::generate_n ( std::back_inserter (preferences)
                  , num_preferences
                  , capability_pool
                  );

  auto const num_workers (fhg::util::testing::random<std::size_t>{} (100, 10));
  auto const num_tasks
     ( fhg::util::testing::random<std::size_t>{} (10, 2)
     * num_workers
     );

  std::vector<sdpa::worker_id_t> test_workers;

  for (unsigned int k (0); k < num_workers; ++k)
  {
    sdpa::worker_id_t const worker (utils::random_peer_name());
    test_workers.emplace_back (worker);
    add_worker ( F::_worker_manager
               , worker
               , { sdpa::Capability (common_capability)
                 , sdpa::Capability
                     (*fhg::util::next (preferences.begin(), k % preferences.size()))
                 }
               );
  }

  for (unsigned int i {0}; i < num_tasks; ++i)
  {
    F::add_and_enqueue_job (require (common_capability, preferences));
  }

  F::_scheduler->assign_jobs_to_workers();

  F::finish_tasks_and_steal_work_when_idle_workers_exist
     (num_tasks, test_workers);
}

MULTIPLE_FIXTURES_TEST_CASE
  ( worker_finishing_tasks_without_preferences_earlier_steals_work
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::GreedyScheduler>
  )
{
  fhg::util::testing::unique_random<std::string> capability_pool;

  std::string const common_capability (capability_pool());

  auto const num_workers (fhg::util::testing::random<std::size_t>{} (100, 10));
  auto const num_tasks
    ( fhg::util::testing::random<std::size_t>{} (10, 2)
    * num_workers
    );

  std::vector<sdpa::worker_id_t> test_workers;
  for (unsigned int k (0); k < num_workers; ++k)
  {
    sdpa::worker_id_t const worker (utils::random_peer_name());
    test_workers.emplace_back (worker);
    add_worker
      (F::_worker_manager, worker, {sdpa::Capability (common_capability)});
  }

  for (unsigned int i {0}; i < num_tasks; ++i)
  {
    F::add_and_enqueue_job (require (common_capability));
  }

  F::_scheduler->assign_jobs_to_workers();

  auto const worker_finishing_tasks_earlier
    (test_workers[fhg::util::testing::random<std::size_t>{} (num_workers - 1)]);

  F::finish_tasks_assigned_to_worker_and_steal_work
    (worker_finishing_tasks_earlier, test_workers);
}

MULTIPLE_FIXTURES_TEST_CASE
  ( worker_finishing_tasks_with_preferences_earlier_steals_work
  , F
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::SingleAllocationScheduler>
  , fixture_scheduler_and_requirements_and_preferences
      <sdpa::daemon::CoallocationScheduler>
  )
{
  fhg::util::testing::unique_random<std::string> capability_pool;

  std::string const common_capability (capability_pool());

  unsigned int const num_preferences (10);
  Preferences preferences;
  std::generate_n ( std::back_inserter (preferences)
                  , num_preferences
                  , capability_pool
                  );

  auto const num_workers (fhg::util::testing::random<std::size_t>{} (100, 10));
  auto const num_tasks
     ( fhg::util::testing::random<std::size_t>{} (10, 2)
     * num_workers
     );

  std::vector<sdpa::worker_id_t> test_workers;
  for (unsigned int k (0); k < num_workers; ++k)
  {
    sdpa::worker_id_t const worker (utils::random_peer_name());
    test_workers.emplace_back (worker);
    add_worker ( F::_worker_manager
               , worker
               , { sdpa::Capability (common_capability)
                 , sdpa::Capability
                     (*fhg::util::next (preferences.begin(), k % preferences.size()))
                 }
               );
  }

  for (unsigned int i {0}; i < num_tasks; ++i)
  {
    F::add_and_enqueue_job (require (common_capability, preferences));
  }

  F::_scheduler->assign_jobs_to_workers();

  auto const worker_finishing_tasks_earlier
    (test_workers[fhg::util::testing::random<std::size_t>{} (num_workers - 1)]);

  F::finish_tasks_assigned_to_worker_and_steal_work
    (worker_finishing_tasks_earlier, test_workers);
}
