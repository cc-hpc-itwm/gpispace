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

#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/WorkerSet.hpp>
#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
#include <sdpa/daemon/scheduler/CostAwareWithWorkStealingStrategy.hpp>
#include <sdpa/daemon/scheduler/Scheduler.hpp>
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

namespace
{
  unsigned long random_ulong()
  {
    return fhg::util::testing::random_integral<unsigned long>();
  }
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

struct fixture_scheduler_and_requirements_and_preferences
{
  using set_workers_t = std::set<sdpa::worker_id_t>;
  using set_jobs_t = std::set<sdpa::job_id_t>;

  fixture_scheduler_and_requirements_and_preferences()
    : _worker_manager()
    , _scheduler
        ( std::make_unique<sdpa::daemon::CoallocationScheduler>
            ( std::bind
                ( &fixture_scheduler_and_requirements_and_preferences::requirements_and_preferences
                , this
                , std::placeholders::_1
                )
            , _worker_manager
            )
        )
    , _access_allocation_table
        (dynamic_cast<sdpa::daemon::CoallocationScheduler*> (_scheduler.get())->strategy_TESTING_ONLY())
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
  const ::boost::optional<unsigned long> no_max_num_retries;

  Requirements_and_preferences require (std::string name, unsigned long workers)
  {

    return { {we::type::Requirement (name)}
           , {workers, no_max_num_retries}
           , null_transfer_cost
           , computational_cost
           , 0
           , {}
           };
  }

  Requirements_and_preferences require (unsigned long workers)
  {
    return { {}
           , {workers, no_max_num_retries}
           , null_transfer_cost
           , computational_cost
           , 0
           , {}
           };
  }

  Requirements_and_preferences require
     ( std::string const& capability
     , unsigned int num_workers
     , Preferences const& preferences
     )
  {
    return { {we::type::Requirement (capability)}
           , {num_workers, no_max_num_retries}
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
}

BOOST_FIXTURE_TEST_CASE
  ( testCoallocSched
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  add_worker (_worker_manager, "A0", {sdpa::Capability ("A")});
  add_worker (_worker_manager, "B0", {sdpa::Capability ("B")});
  add_worker (_worker_manager, "A1", {sdpa::Capability ("A")});
  add_worker (_worker_manager, "B1", {sdpa::Capability ("B")});

  auto const job_2a (add_and_enqueue_job (require ("A", 2)));
  auto const job_2b (add_and_enqueue_job (require ("B", 2)));

  {
    _scheduler->assign_jobs_to_workers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL (assignment.at (job_2a), set_workers_t ({"A0", "A1"}));
    BOOST_REQUIRE_EQUAL (assignment.at (job_2b), set_workers_t ({"B0", "B1"}));
  }

  auto const job_1a (add_and_enqueue_job (require ("A", 1)));

  {
    _scheduler->assign_jobs_to_workers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE ( assignment.at (job_1a) == set_workers_t ({"A0"})
                 || assignment.at (job_1a) == set_workers_t ({"A1"})
                  );
  }
}

BOOST_FIXTURE_TEST_CASE
  ( not_schedulable_job_does_not_block_others
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  add_worker (_worker_manager, "worker");

  add_and_enqueue_job (require (2));

  {
    _scheduler->assign_jobs_to_workers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE (assignment.empty());
  }

  auto const job (add_and_enqueue_job (require (1)));

  {
    _scheduler->assign_jobs_to_workers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL ( assignment.at (job)
                        , set_workers_t ({"worker"})
                        );
  }

  _scheduler->start_pending_jobs (SERVE_JOB_AND_CHECK_EXPECTED (job));
}

struct serve_job_and_check_for_minimal_cost_assignement
{
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
    ( std::function<double (std::string const&)> cost
    , std::vector<std::string> const& worker_ids
    , sdpa::daemon::WorkerSet const& assigned_workers
    , sdpa::daemon::Implementation const&
    , sdpa::job_id_t const&
    )
  {
    sdpa::worker_id_t assigned_worker_with_max_cost
      (*std::max_element ( assigned_workers.begin()
                         , assigned_workers.end()
                         , [cost] ( sdpa::worker_id_t const& left
                                  , sdpa::worker_id_t const& right
                                  )
                           { return cost (left) < cost (right); }
                         )
      );

    for (sdpa::worker_id_t const& wid : worker_ids)
    {
      if (!assigned_workers.count (wid))
      {
         // any not assigned worker has n associated a cost that is either greater or equal
         // to the maximum cost of the assigned workers
         BOOST_REQUIRE_GE (cost (wid), cost (assigned_worker_with_max_cost));
      }
    }
  }
};

BOOST_FIXTURE_TEST_CASE ( scheduling_with_data_locality_and_random_costs
                        , serve_job_and_check_for_minimal_cost_assignement
                        )
{
  auto const n_workers
    (fhg::util::testing::random<unsigned int>{} (20, 10));
  const unsigned int n_req_workers (10);

  const std::vector<std::string>
    worker_ids (generate_worker_names (n_workers));

  const std::map<sdpa::worker_id_t, double>
    map_transfer_costs (generate_costs (worker_ids));

  const std::function<double (std::string const&)>
    test_transfer_cost ( [&map_transfer_costs](std::string const& worker) -> double
                         {
                           return map_transfer_costs.at (worker);
                         }
                       );

  sdpa::daemon::WorkerManager worker_manager;
  auto const _scheduler
    ( std::make_unique<sdpa::daemon::CoallocationScheduler>
        ( [&test_transfer_cost] (sdpa::job_id_t const&)
          {
            return Requirements_and_preferences
              ( {}
              , {n_req_workers, no_max_num_retries}
              , test_transfer_cost
              , computational_cost
              , 0
              , {}
              );
          }
        , worker_manager
        )
    );

  fhg::util::testing::random<unsigned short> const random_ushort;

  for (sdpa::worker_id_t const& worker_id : worker_ids)
  {
    std::lock_guard<std::mutex> lock (worker_manager._mutex);
    worker_manager.add_worker
      ( worker_id
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
  _scheduler->submit_job (job_id);

  _scheduler->assign_jobs_to_workers();
  _scheduler->start_pending_jobs
    ( [this, &job_id, &test_transfer_cost, &worker_ids]
      ( sdpa::daemon::WorkerSet const& assigned_workers
      , sdpa::daemon::Implementation const& implementation
      , sdpa::job_id_t const& started_job
      , std::function<fhg::com::p2p::address_t (sdpa::worker_id_t const&)>
      )
      {
        BOOST_REQUIRE_EQUAL (job_id, started_job);

        serve_and_check_assignment
          ( test_transfer_cost
          , worker_ids
          , assigned_workers
          , implementation
          , started_job
         );
      }
    );
}

BOOST_AUTO_TEST_CASE (scheduling_bunch_of_jobs_with_preassignment_and_load_balancing)
{
  const unsigned int n_jobs (1000);
  const unsigned int n_req_workers (1);

  const int n_workers (20);
  std::vector<sdpa::worker_id_t> worker_ids (n_workers);
  std::generate_n (worker_ids.begin(), n_workers, utils::random_peer_name);

  const int n_hosts (n_workers); // assume workers are running on distinct hosts
  std::vector<sdpa::worker_id_t> host_ids (n_hosts);
  std::generate_n (host_ids.begin(), n_hosts, utils::random_peer_name);

  std::uniform_real_distribution<double> dist (1.0, n_hosts);
  auto& rand_engine (fhg::util::testing::detail::GLOBAL_random_engine());

  const double _computational_cost (dist (rand_engine));
  std::vector<double> transfer_costs (n_hosts);
  std::generate_n ( transfer_costs.begin()
                  , n_hosts
                  , [&dist,&rand_engine]() {return dist (rand_engine);}
                  );

  const std::function<double (std::string const&)>
    test_transfer_cost ( [&host_ids, &transfer_costs]
                         (std::string const& host) -> double
                         {
                           std::vector<std::string>::const_iterator
                             it (std::find (host_ids.begin(), host_ids.end(), host));
                           if (it != host_ids.end())
                           {
                             return  transfer_costs[it - host_ids.begin()];
                           }
                           else
                           {
                             throw std::runtime_error ("Unexpected host argument in transfer cost function");
                           }
                         }
                       );

  sdpa::daemon::WorkerManager worker_manager;
  auto const _scheduler
    ( std::make_unique<sdpa::daemon::CoallocationScheduler>
        ( [&test_transfer_cost, &_computational_cost]
          (sdpa::job_id_t const&)
          {
            return Requirements_and_preferences
              ( {}
              , {n_req_workers, no_max_num_retries}
              , test_transfer_cost
              , _computational_cost
              , 0
              , {}
              );
          }
        , worker_manager
        )
    );

  fhg::util::testing::random<unsigned short> const random_ushort;

  for (decltype (worker_ids)::size_type i=0; i<n_workers;i++)
  {
    std::lock_guard<std::mutex> lock (worker_manager._mutex);
    worker_manager.add_worker
      ( worker_ids[i]
      , {}
      , random_ulong()
      , host_ids[i]
      , fhg::com::p2p::address_t
          { fhg::com::host_t {host_ids.at (i)}
          , fhg::com::port_t {random_ushort()}
          }
      );
  }

  fhg::util::testing::unique_random<sdpa::job_id_t> job_ids;
  for (unsigned int i (0); i < n_jobs; ++i)
  {
    _scheduler->submit_job (job_ids());
  }

  _scheduler->assign_jobs_to_workers();
  auto const allocation_table
    (sdpa::daemon::access_allocation_table_TESTING_ONLY (_scheduler.get()->strategy_TESTING_ONLY()));
  auto const assignment
    (allocation_table. get_current_assignment());

  std::vector<double> sum_costs_assigned_jobs (n_workers, 0.0);
  const double max_job_cost ( *std::max_element (transfer_costs.begin(), transfer_costs.end())
                            + _computational_cost
                            );

  for ( std::set<sdpa::worker_id_t> const& job_assigned_workers
      : assignment | ::boost::adaptors::map_values
      )
  {
    for (decltype (worker_ids)::size_type k=0; k<n_workers; ++k)
    {
      sum_costs_assigned_jobs[k] += job_assigned_workers.count (worker_ids[k])
                                  * (transfer_costs[k] + _computational_cost);
    }
  }

  BOOST_REQUIRE_LE ( std::abs ( *std::max_element (sum_costs_assigned_jobs.begin(), sum_costs_assigned_jobs.end())
                              - *std::min_element (sum_costs_assigned_jobs.begin(), sum_costs_assigned_jobs.end())
                              )
                   , max_job_cost
                   );
}

BOOST_FIXTURE_TEST_CASE
  ( no_assignment_if_not_enough_memory
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  unsigned long avail_mem (random_ulong());
  if (avail_mem > 0) avail_mem--;

  fhg::util::testing::random<std::string> const random_string;
  fhg::util::testing::random<unsigned short> const random_ushort;

  auto const hostname {random_string()};

  add_worker
    ( _worker_manager
    , "worker_0"
    , {}
    , avail_mem
    , hostname
    , fhg::com::p2p::address_t
        { fhg::com::host_t {hostname}
        , fhg::com::port_t {random_ushort()}
        }
    );

  add_and_enqueue_job
    ( Requirements_and_preferences ( {}
                                   , we::type::schedule_data()
                                   , null_transfer_cost
                                   , computational_cost
                                   , avail_mem + 1
                                   , {}
                                   )
    );

  _scheduler->assign_jobs_to_workers();

  BOOST_REQUIRE (get_current_assignment().empty());
}

BOOST_FIXTURE_TEST_CASE ( invariant_assignment_for_jobs_with_different_memory_requirements_and_preferences
                        , fixture_scheduler_and_requirements_and_preferences
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
      ( _worker_manager
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
      ( _worker_manager
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
      ( _worker_manager
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
    ( add_and_enqueue_job
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
    ( add_and_enqueue_job
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
    _scheduler->assign_jobs_to_workers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());
    BOOST_REQUIRE (assignment.count (job_id_0));
    BOOST_REQUIRE (assignment.count (job_id_1));

    BOOST_REQUIRE_EQUAL (assignment.at (job_id_0), set_0);
    BOOST_REQUIRE_EQUAL (assignment.at (job_id_1), set_1);
  }

  _scheduler->release_reservation (job_id_0);
  _scheduler->release_reservation (job_id_1);

  _scheduler->submit_job (job_id_1);
  _scheduler->submit_job (job_id_0);

  {
    _scheduler->assign_jobs_to_workers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());
    BOOST_REQUIRE (assignment.count (job_id_0));
    BOOST_REQUIRE (assignment.count (job_id_1));

    BOOST_REQUIRE_EQUAL (assignment.at (job_id_0), set_0);
    BOOST_REQUIRE_EQUAL (assignment.at (job_id_1), set_1);
  }
}

BOOST_FIXTURE_TEST_CASE ( assign_to_the_same_worker_if_the_total_cost_is_lower
                        , fixture_scheduler_and_requirements_and_preferences
                        )
{
  std::string const name_worker_0 {utils::random_peer_name()};
  std::string const name_worker_1 {utils::random_peer_name()};
  std::string const name_node_0 {utils::random_peer_name()};
  std::string const name_node_1 {utils::random_peer_name()};

  std::set<sdpa::worker_id_t> const expected_assignment {name_worker_1};

  fhg::util::testing::random<unsigned short> const random_ushort;

  add_worker
    ( _worker_manager
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
      ( _worker_manager
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
      ( add_and_enqueue_job
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

  _scheduler->assign_jobs_to_workers();
  auto const assignment (get_current_assignment());

  BOOST_REQUIRE (!assignment.empty());
  for (auto const& job_id : jobs)
  {
    BOOST_REQUIRE (assignment.count (job_id));
  }

  // No worker should be idle and in the case when new jobs are added, these
  // should be assigned to the worker that minimizes the total cost.
  // In this test case, they are assigned to worker_1, because the transfer
  // cost is much smaller for this, any time a new job is added,
  BOOST_REQUIRE_EQUAL (count_assigned_jobs (assignment, name_worker_0), 1);
  BOOST_REQUIRE_EQUAL
    (count_assigned_jobs (assignment, name_worker_1), assignment.size() - 1);
}

struct fixture_add_new_workers
{
  fixture_add_new_workers()
    : _worker_manager()
    , _scheduler
        ( std::make_unique<sdpa::daemon::CoallocationScheduler>
            ( std::bind ( &fixture_add_new_workers::requirements_and_preferences
                        , this
                        , std::placeholders::_1
                        )
            , _worker_manager
            )
        )
    , _access_allocation_table
         (dynamic_cast<sdpa::daemon::CoallocationScheduler*> (_scheduler.get())->strategy_TESTING_ONLY())
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

      BOOST_REQUIRE (n_jobs_diff == 0 || n_jobs_diff == 1);

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

BOOST_FIXTURE_TEST_CASE
  (request_arbitrary_number_of_workers, fixture_add_new_workers)
{
  unsigned int const n_workers (1000);
  unsigned int const n_max_workers_per_job (10);

  std::vector<sdpa::worker_id_t> const workers
    (add_new_workers ({"A"}, n_workers));

  unsigned int n_assigned_workers (0);
  while (n_assigned_workers < n_workers)
  {
    auto const n_req_workers
      (fhg::util::testing::random<unsigned int>{} (n_max_workers_per_job, 1));

    auto const job (add_and_enqueue_job (require ("A", n_req_workers)));

    request_scheduling();

    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL (assignment.at (job).size(), n_req_workers);

    n_assigned_workers += n_req_workers;
  }
}

BOOST_FIXTURE_TEST_CASE
  ( request_arbitrary_number_of_workers_and_finish_arbitrary_number_of_jobs
  , fixture_add_new_workers
  )
{
  unsigned int const n_workers (1000);
  unsigned int const n_jobs (1000);
  unsigned int const n_max_workers_per_job (10);

  std::vector<sdpa::worker_id_t> const workers
    (add_new_workers ({"A"}, n_workers));

  for (unsigned int k = 0; k < n_jobs; k++)
  {
    auto const n_req_workers
      (fhg::util::testing::random<unsigned int>{} (n_max_workers_per_job, 1));

    add_and_enqueue_job (require ("A", n_req_workers));
  }

  _scheduler->assign_jobs_to_workers();

  auto assignment (get_current_assignment());

  for (auto const& req : _requirements_and_preferences)
  {
    BOOST_REQUIRE_EQUAL
      ( assignment.at (req.first).size()
      , req.second.numWorkers()
      );
  }

  unsigned int started_jobs (0);

  std::set<sdpa::job_id_t> running_jobs;
  while (!assignment.empty())
  {
    std::set<sdpa::job_id_t> jobs_started;
    _scheduler->start_pending_jobs
      ( [this, &started_jobs, &jobs_started]
           ( sdpa::daemon::WorkerSet const& assigned_workers
           , sdpa::daemon::Implementation const& implementation
           , sdpa::job_id_t const& job
           , std::function<fhg::com::p2p::address_t (sdpa::worker_id_t const&)>
           )
        {
          BOOST_REQUIRE_EQUAL
            ( this->workers (job).size()
            , _requirements_and_preferences.at (job).numWorkers()
            );

          BOOST_REQUIRE_EQUAL (this->workers (job), assigned_workers);
          BOOST_REQUIRE_EQUAL (this->implementation (job), implementation);

          jobs_started.emplace (job);
          ++started_jobs;
        }
      );

    running_jobs.insert (jobs_started.begin(), jobs_started.end());
    if (!running_jobs.empty())
    {
      auto const n_finishing_jobs
        (fhg::util::testing::random<std::size_t>{} (running_jobs.size(), 1));

      // finish an arbitrary number of running jobs
      for (std::size_t k (0); k < n_finishing_jobs; k++)
      {
        _scheduler->release_reservation (*running_jobs.begin());
        running_jobs.erase (running_jobs.begin());
      }
    }

    _scheduler->assign_jobs_to_workers();
    assignment = get_current_assignment();
  }

  BOOST_REQUIRE_EQUAL (started_jobs, n_jobs);
}

BOOST_FIXTURE_TEST_CASE
  ( coallocation_with_multiple_implementations_is_forbidden
  , fixture_add_new_workers
  )
{
  fhg::util::testing::unique_random<std::string> capability_pool;

  std::string const common_capability (capability_pool());

  std::string preference (capability_pool());

  auto const num_workers
    (fhg::util::testing::random<unsigned int>{} (4, 2));

  std::vector<sdpa::worker_id_t> const workers
    (add_new_workers ( {common_capability, preference}
                     , num_workers
                     )
    );

  add_and_enqueue_job
    ( require ( common_capability
              , num_workers
              , {preference, capability_pool(), capability_pool()}
              )
    );

  fhg::util::testing::require_exception
     ( [this] { _scheduler->assign_jobs_to_workers(); }
     ,  std::runtime_error
         ("Coallocation with preferences is forbidden!")
     );
}
