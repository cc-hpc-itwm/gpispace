#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>

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
  std::string (&random_job_id)(void) = utils::random_peer_name;
  auto serve_job = [] ( sdpa::daemon::WorkerSet const&
                      , sdpa::daemon::Implementation const&
                      , const sdpa::job_id_t&
                      )
                   {};

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
      access_allocation_table_TESTING_ONLY (CoallocationScheduler& _scheduler)
        : _allocation_table (_scheduler.allocation_table_)
      {}
      std::map<job_id_t, std::set<worker_id_t>> get_current_assignment() const
      {
        std::map<job_id_t, std::set<worker_id_t>> assignment;
        std::transform
          ( _allocation_table.begin()
          , _allocation_table.end()
          , std::inserter (assignment, assignment.end())
          , [] (CoallocationScheduler::allocation_table_t::value_type const& p)
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
      CoallocationScheduler::allocation_table_t& _allocation_table;
    };
  }
}

struct fixture_scheduler_and_requirements_and_preferences
{
  typedef std::set<sdpa::worker_id_t> set_workers_t;
  typedef std::set<sdpa::job_id_t> set_jobs_t;

  fixture_scheduler_and_requirements_and_preferences()
    : _worker_manager()
    , _scheduler
      ( std::bind ( &fixture_scheduler_and_requirements_and_preferences::requirements_and_preferences
                  , this
                  , std::placeholders::_1
                  )
      , _worker_manager
      )
    , _access_allocation_table (_scheduler)
  {}

  sdpa::daemon::WorkerManager _worker_manager;
  sdpa::daemon::CoallocationScheduler _scheduler;
  sdpa::daemon::access_allocation_table_TESTING_ONLY _access_allocation_table;

  ~fixture_scheduler_and_requirements_and_preferences()
  {
  }

  std::map<sdpa::job_id_t, std::set<sdpa::worker_id_t>> get_current_assignment() const
  {
    return _access_allocation_table.get_current_assignment();
  }

  sdpa::daemon::WorkerSet const workers (sdpa::job_id_t const& job) const
  {
    return _access_allocation_table.workers (job);
  }

  sdpa::daemon::Implementation const implementation (sdpa::job_id_t const& job) const
  {
    return _access_allocation_table.implementation (job);
  }

  void add_job ( const sdpa::job_id_t& job_id
               , const Requirements_and_preferences& reqs_and_prefs
               )
  {
    if (!_requirements_and_preferences.emplace (job_id, reqs_and_prefs).second)
    {
      throw std::runtime_error ("added job twice");
    }
  }

  Requirements_and_preferences requirements_and_preferences (sdpa::job_id_t id)
  {
    return _requirements_and_preferences.find (id)->second;
  }

  std::map<sdpa::job_id_t, Requirements_and_preferences> _requirements_and_preferences;

  unsigned long count_assigned_jobs
    ( std::map<sdpa::job_id_t, std::set<sdpa::worker_id_t>> assignment
    , const sdpa::worker_id_t& worker_id
    )
  {
    auto value = [](std::pair<sdpa::job_id_t, std::set<sdpa::worker_id_t>>const& p)
                   {
                     return p.second;
                   };


    return (std::count_if ( boost::make_transform_iterator (assignment.begin(), value)
                          , boost::make_transform_iterator (assignment.end(), value)
                          , [&worker_id] (const std::set<sdpa::worker_id_t>& v)
                            {
                              return v.count (worker_id);
                            }
                          )
           );
  }

  void require_worker_and_implementation
    ( sdpa::job_id_t const& job
    , sdpa::worker_id_t const& worker
    , boost::optional<std::string> const& impl
    )
  {
    auto const assignment (get_current_assignment());
    BOOST_REQUIRE (assignment.count (job));

    BOOST_REQUIRE_EQUAL (sdpa::daemon::WorkerSet {worker}, workers (job));
    BOOST_REQUIRE_EQUAL (impl, implementation (job));
  }
};

namespace
{
  const double computational_cost = 1.0;

  Requirements_and_preferences require (std::string name_1)
  {
    return { {we::type::requirement_t (name_1, true)}
           , we::type::schedule_data()
           , null_transfer_cost
           , computational_cost
           , 0
           , {}
           };
  }

  Requirements_and_preferences require (std::string name, unsigned long workers)
  {
    return { {we::type::requirement_t (name, true)}
           , we::type::schedule_data (workers)
           , null_transfer_cost
           , computational_cost
           , 0
           , {}
           };
  }

  Requirements_and_preferences require (unsigned long workers)
  {
    return { {}
           , we::type::schedule_data (workers)
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
    return { {we::type::requirement_t (capability, true)}
           , we::type::schedule_data()
           , null_transfer_cost
           , computational_cost
           , 0
           , preferences
           };
  }

  Requirements_and_preferences require
     ( std::string const& capability
     , unsigned int num_workers
     , Preferences const& preferences
     )
  {
    return { {we::type::requirement_t (capability, true)}
           , we::type::schedule_data (num_workers)
           , null_transfer_cost
           , computational_cost
           , 0
           , preferences
           };
  }
}

BOOST_FIXTURE_TEST_CASE
  ( load_balancing
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  _worker_manager.addWorker ( "worker_0"
                                        , {}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  _worker_manager.addWorker ( "worker_1"
                                        , {}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  const unsigned int n_jobs (100);
  std::vector<sdpa::job_id_t> job_ids (n_jobs);
  std::generate_n (job_ids.begin(), n_jobs, random_job_id);

  for (sdpa::job_id_t job_id : job_ids)
  {
    add_job (job_id, no_requirements_and_preferences());
    _scheduler.enqueueJob (job_id);
  }

  _scheduler.assignJobsToWorkers();
  auto const assignment (get_current_assignment());

  unsigned long const assigned_to_worker_0
    (count_assigned_jobs (assignment, "worker_0"));
  unsigned long const assigned_to_worker_1
    (count_assigned_jobs (assignment, "worker_1"));

  BOOST_REQUIRE (  (assigned_to_worker_0 <= assigned_to_worker_1 + 1UL)
                && (assigned_to_worker_1 <= assigned_to_worker_0 + 1UL)
                );
}

BOOST_FIXTURE_TEST_CASE
  ( tesLBOneWorkerJoinsLater
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  _worker_manager.addWorker ( "worker_0"
                                        , {}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  add_job ("job_0", no_requirements_and_preferences());
  add_job ("job_1", no_requirements_and_preferences());

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");

  {
    _scheduler.assignJobsToWorkers();
    _scheduler.steal_work();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL (count_assigned_jobs (assignment, "worker_0"), 2);
  }

  _worker_manager.addWorker ( "worker_1"
                            , {}
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  {
    _scheduler.assignJobsToWorkers();
    _scheduler.steal_work();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL (count_assigned_jobs (assignment, "worker_0"), 1);
    BOOST_REQUIRE_EQUAL (count_assigned_jobs (assignment, "worker_1"), 1);
  }
}


BOOST_FIXTURE_TEST_CASE
  ( tesLBOneWorkerGainsCpbLater
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  _worker_manager.addWorker ( "worker_0"
                            , {sdpa::capability_t ("C", "worker_0")}
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  _worker_manager.addWorker ( "worker_1"
                            , {}
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  add_job ("job_0", require ("C"));
  add_job ("job_1", require ("C"));

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");

  {
    _scheduler.assignJobsToWorkers();
    _scheduler.steal_work();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL (count_assigned_jobs (assignment, "worker_0"), 2);
  }

  _worker_manager.add_worker_capabilities ("worker_1", {sdpa::capability_t ("C", "worker_1")});

  {
    _scheduler.assignJobsToWorkers();
    _scheduler.steal_work();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL (count_assigned_jobs (assignment, "worker_0"), 1);
    BOOST_REQUIRE_EQUAL (count_assigned_jobs (assignment, "worker_1"), 1);
  }
}

BOOST_FIXTURE_TEST_CASE
  ( testCoallocSched
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  _worker_manager.addWorker ( "A0"
                                        , {sdpa::capability_t ("A", "A0")}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  _worker_manager.addWorker ( "B0"
                                        , {sdpa::capability_t ("B", "B0")}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  _worker_manager.addWorker ( "A1"
                                        , {sdpa::capability_t ("A", "A1")}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  _worker_manager.addWorker ( "B1"
                                        , {sdpa::capability_t ("B", "B1")}
                                        , random_ulong()
                                        , false, fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  add_job ("2A", require ("A", 2));
  add_job ("2B", require ("B", 2));

  _scheduler.enqueueJob ("2A");
  _scheduler.enqueueJob ("2B");

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL (assignment.at ("2A"), set_workers_t ({"A0", "A1"}));
    BOOST_REQUIRE_EQUAL (assignment.at ("2B"), set_workers_t ({"B0", "B1"}));
  }

  add_job ("1A", require ("A", 1));

  _scheduler.enqueueJob ("1A");

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE ( assignment.at ("1A") == set_workers_t ({"A0"})
                 || assignment.at ("1A") == set_workers_t ({"A1"})
                  );
  }
}

BOOST_FIXTURE_TEST_CASE
  ( tesLBStopRestartWorker
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  _worker_manager.addWorker ( "worker_0"
                            , {}
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  _worker_manager.addWorker ( "worker_1"
                            , {}
                            , random_ulong()
                            , false, fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  add_job ("job_0", no_requirements_and_preferences());
  add_job ("job_1", no_requirements_and_preferences());

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");

  _scheduler.assignJobsToWorkers();
  auto const assignment (get_current_assignment());

  BOOST_REQUIRE ( assignment.at ("job_0") == set_workers_t ({"worker_0"})
               || assignment.at ("job_0") == set_workers_t ({"worker_1"})
                );

  BOOST_REQUIRE ( assignment.at ("job_1") == set_workers_t ({"worker_0"})
               || assignment.at ("job_1") == set_workers_t ({"worker_1"})
                );

  BOOST_REQUIRE( assignment.at ("job_0") !=  assignment.at ("job_1"));

  sdpa::job_id_t job_assigned_to_worker_0 ( assignment.at ("job_0") == set_workers_t ({"worker_0"})
                                          ? "job_0"
                                          : "job_1"
                                          );

  _scheduler.releaseReservation (job_assigned_to_worker_0);
  _worker_manager.deleteWorker ("worker_0");
  _scheduler.enqueueJob (job_assigned_to_worker_0);

  _worker_manager.addWorker ( "worker_0"
                            , {}
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  _scheduler.assignJobsToWorkers();
  auto const new_assignment (get_current_assignment());

  BOOST_REQUIRE_EQUAL ( new_assignment.at (job_assigned_to_worker_0)
                      , set_workers_t ({"worker_0"})
                      );
}

BOOST_FIXTURE_TEST_CASE
  ( not_schedulable_job_does_not_block_others
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  _worker_manager.addWorker ( "worker"
                            , {}
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  add_job ("2", require (2));
  _scheduler.enqueueJob ("2");

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE (assignment.empty());
  }

  add_job ("1", require (1));
  _scheduler.enqueueJob ("1");

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL ( assignment.at ("1")
                        , set_workers_t ({"worker"})
                        );
  }

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({"1"})
                      );
}

BOOST_FIXTURE_TEST_CASE
  ( multiple_job_submissions_no_requirements_and_preferences
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  _worker_manager.addWorker ( worker_id
                            , {}
                            , random_ulong()
                            , true
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  sdpa::job_id_t const job_id_0 (random_job_id());
  add_job (job_id_0, no_requirements_and_preferences());
  _scheduler.enqueueJob (job_id_0);

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL ( assignment.at (job_id_0)
                        , set_workers_t ({worker_id})
                        );
  }

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_0})
                      );

  sdpa::job_id_t const job_id_1 (random_job_id());
  add_job (job_id_1, no_requirements_and_preferences());
  _scheduler.enqueueJob (job_id_1);

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL ( assignment.at (job_id_1)
                        , set_workers_t ({worker_id})
                        );
  }

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_1})
                      );
}

BOOST_FIXTURE_TEST_CASE ( multiple_job_submissions_with_no_children_allowed
                        , fixture_scheduler_and_requirements_and_preferences
                        )
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  _worker_manager.addWorker ( worker_id
                                        , {}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  sdpa::job_id_t const job_id_0 (random_job_id());
  add_job (job_id_0, no_requirements_and_preferences());
  _scheduler.enqueueJob (job_id_0);

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL ( assignment.at (job_id_0)
                        , set_workers_t ({worker_id})
                        );
  }

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_0})
                      );

  sdpa::job_id_t const job_id_1 (random_job_id());
  add_job (job_id_1, no_requirements_and_preferences());
  _scheduler.enqueueJob (job_id_1);

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL ( assignment.at (job_id_1)
                        , set_workers_t ({worker_id})
                        );
  }

  BOOST_REQUIRE (_scheduler.start_pending_jobs (serve_job).empty());

  _scheduler.releaseReservation (job_id_0);

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                       , set_jobs_t ({job_id_1})
                       );
}

BOOST_FIXTURE_TEST_CASE
  ( multiple_worker_job_submissions_with_requirements_and_preferences
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  _worker_manager.addWorker ( worker_id
                            , { sdpa::capability_t("A", worker_id)
                              , sdpa::capability_t("B", worker_id)
                              }
                            , random_ulong()
                            , true
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  sdpa::job_id_t const job_id_0 (random_job_id());
  add_job (job_id_0, require ("A"));
  _scheduler.enqueueJob (job_id_0);

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL ( assignment.at (job_id_0)
                        , set_workers_t ({worker_id})
                        );
  }

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_0})
                      );

  sdpa::job_id_t const job_id_1 (random_job_id());
  add_job (job_id_1, require ("B"));
  _scheduler.enqueueJob (job_id_1);

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL ( assignment.at (job_id_1)
                        , set_workers_t ({worker_id})
                        );
  }

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_1})
                      );
}

BOOST_FIXTURE_TEST_CASE
  ( multiple_worker_job_submissions_with_requirements_and_preferences_no_children_allowed
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  _worker_manager.addWorker ( worker_id
                            , { sdpa::capability_t ("A", worker_id)
                              , sdpa::capability_t ("B", worker_id)
                              }
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  sdpa::job_id_t const job_id_0 (utils::random_peer_name());
  add_job (job_id_0, require ("A"));
  _scheduler.enqueueJob (job_id_0);

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

     BOOST_REQUIRE_EQUAL ( assignment.at (job_id_0)
                         , set_workers_t ({worker_id})
                         );
  }

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_0})
                      );

  sdpa::job_id_t const job_id_1 (utils::random_peer_name());
  add_job (job_id_1, require ("B"));
  _scheduler.enqueueJob (job_id_1);

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL ( assignment.at (job_id_1)
                        , set_workers_t ({worker_id})
                        );
  }

   BOOST_REQUIRE (_scheduler.start_pending_jobs (serve_job).empty());

  _scheduler.releaseReservation (job_id_0);

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_1})
                      );
}

struct fixture_minimal_cost_assignment
{
  fixture_minimal_cost_assignment()
  : _worker_manager()
  , _scheduler
      ( [](const sdpa::job_id_t&) {return no_requirements_and_preferences();}
      , _worker_manager
      )
  {
    _worker_manager.addWorker ("worker_01", {}, random_ulong(), false, "node1", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_02", {}, random_ulong(), false, "node1", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_03", {}, random_ulong(), false, "node1", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_04", {}, random_ulong(), false, "node1", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_05", {}, random_ulong(), false, "node2", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_06", {}, random_ulong(), false, "node2", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_07", {}, random_ulong(), false, "node2", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_08", {}, random_ulong(), false, "node2", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_09", {}, random_ulong(), false, "node3", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_10", {}, random_ulong(), false, "node3", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_11", {}, random_ulong(), false, "node3", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_12", {}, random_ulong(), false, "node3", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_13", {}, random_ulong(), false, "node4", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_14", {}, random_ulong(), false, "node4", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_15", {}, random_ulong(), false, "node4", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_16", {}, random_ulong(), false, "node4", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_17", {}, random_ulong(), false, "node5", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_18", {}, random_ulong(), false, "node5", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_19", {}, random_ulong(), false, "node5", fhg::util::testing::random_string());
    _worker_manager.addWorker ("worker_20", {}, random_ulong(), false, "node5", fhg::util::testing::random_string());
  }

  double max_value (const std::map<std::string, double>& map_cost)
  {
    return std::max_element ( map_cost.begin()
                            , map_cost.end()
                            , [] ( const std::pair<std::string, double>& lhs
                                 , const std::pair<std::string, double>& rhs
                                 )
                                 {return lhs.second < rhs.second;}
                            )->second;
  }

  sdpa::daemon::WorkerManager _worker_manager;
  sdpa::daemon::CoallocationScheduler _scheduler;
};

struct serve_job_and_check_for_minimal_cost_assignement
{
  unsigned int generate_number_of_workers ( const unsigned int n_min
                                          , const unsigned int n_max
                                          )
  {
    std::random_device rd;
    std::mt19937 gen (rd());
    return std::uniform_int_distribution<> (n_min, n_max) (gen);
  }

  std::vector<std::string> generate_worker_names (const unsigned int n)
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
    std::random_device rd;
    std::mt19937 gen (rd());
    std::normal_distribution<> dist(0,1);

    std::map<sdpa::worker_id_t, double> map_costs;
    for (sdpa::worker_id_t const& worker : worker_ids)
    {
      map_costs.insert (std::make_pair (worker, dist (gen)));
    }

    return map_costs;
  }

  void serve_and_check_assignment
    ( const std::function<double (std::string const&)> cost
    , const std::vector<std::string>& worker_ids
    , sdpa::daemon::WorkerSet const& assigned_workers
    , sdpa::daemon::Implementation const&
    , const sdpa::job_id_t&
    )
  {
    sdpa::worker_id_t assigned_worker_with_max_cost
      (*std::max_element ( assigned_workers.begin()
                         , assigned_workers.end()
                         , [cost] ( const sdpa::worker_id_t& left
                                  , const sdpa::worker_id_t& right
                                  )
                           { return cost (left) < cost (right); }
                         )
      );

    for (const sdpa::worker_id_t& wid : worker_ids)
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
  const unsigned int n_min_workers = 10;
  const unsigned int n_max_workers = 20;
  const unsigned int n_workers (generate_number_of_workers (n_min_workers, n_max_workers));
  const unsigned int n_req_workers (10);

  const std::vector<std::string>
    worker_ids (generate_worker_names (n_workers));

  const std::map<sdpa::worker_id_t, double>
    map_transfer_costs (generate_costs (worker_ids));

  const std::function<double (std::string const&)>
    test_transfer_cost ( [&map_transfer_costs](const std::string& worker) -> double
                         {
                           return map_transfer_costs.at (worker);
                         }
                       );

  sdpa::daemon::WorkerManager _worker_manager;
  sdpa::daemon::CoallocationScheduler
    _scheduler (  [&test_transfer_cost] (const sdpa::job_id_t&)
                  {
                    return Requirements_and_preferences
                      ( {}
                      , we::type::schedule_data (n_req_workers)
                      , test_transfer_cost
                      , computational_cost
                      , 0
                      , {}
                      );
                  }
               , _worker_manager
               );

  for (sdpa::worker_id_t const& worker_id : worker_ids)
  {
    _worker_manager.addWorker (worker_id, {}, random_ulong(), false, worker_id, fhg::util::testing::random_string());
  }

  const sdpa::job_id_t job_id (fhg::util::testing::random_string());
  _scheduler.enqueueJob (job_id);

  _scheduler.assignJobsToWorkers();
  _scheduler.start_pending_jobs
    (std::bind ( &serve_job_and_check_for_minimal_cost_assignement::serve_and_check_assignment
               , this
               , test_transfer_cost
               , worker_ids
               , std::placeholders::_1
               , std::placeholders::_2
               , std::placeholders::_3
               )
    );

  // require the job to be scheduled
  BOOST_REQUIRE_EQUAL (_scheduler.delete_job (job_id), 0);
}

BOOST_FIXTURE_TEST_CASE
  ( no_coallocation_job_with_requirements_and_preferences_is_assigned_if_not_all_workers_are_leaves
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  sdpa::worker_id_t const agent_id (utils::random_peer_name());

  _worker_manager.addWorker ( agent_id
                                        , {sdpa::capability_t ("A", utils::random_peer_name())}
                                        , random_ulong()
                                        , true
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  sdpa::worker_id_t const worker_id (utils::random_peer_name());
  _worker_manager.addWorker ( worker_id
                                        , {sdpa::capability_t ("A", worker_id)}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  sdpa::job_id_t const job_id_0 (utils::random_peer_name());
  add_job (job_id_0, require ("A", 2));
  _scheduler.enqueueJob (job_id_0);

  // no serveJob expected
  _scheduler.assignJobsToWorkers();
  _scheduler.start_pending_jobs (serve_job);

  BOOST_REQUIRE (_scheduler.delete_job (job_id_0));
}

BOOST_FIXTURE_TEST_CASE
  ( no_coallocation_job_without_requirements_and_preferences_is_assigned_if_not_all_workers_are_leaves
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  sdpa::worker_id_t const agent_id (utils::random_peer_name());
  _worker_manager.addWorker ( agent_id
                                        , {}
                                        , random_ulong()
                                        , true
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  sdpa::worker_id_t const worker_id (utils::random_peer_name());
  _worker_manager.addWorker ( worker_id
                                        , {}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  sdpa::job_id_t const job_id_0 (utils::random_peer_name());

  add_job (job_id_0, require (2));
  _scheduler.enqueueJob (job_id_0);

  // no serveJob expected
  _scheduler.assignJobsToWorkers();
  _scheduler.start_pending_jobs (serve_job);

  BOOST_REQUIRE (_scheduler.delete_job (job_id_0));
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
  std::random_device rand_dev;
  std::mt19937 rand_engine (rand_dev());

  const double _computational_cost (dist (rand_engine));
  std::vector<double> transfer_costs (n_hosts);
  std::generate_n ( transfer_costs.begin()
                  , n_hosts
                  , [&dist,&rand_engine]() {return dist (rand_engine);}
                  );

  const std::function<double (std::string const&)>
    test_transfer_cost ( [&host_ids, &transfer_costs]
                         (const std::string& host) -> double
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

  std::vector<sdpa::job_id_t> job_ids (n_jobs);
  std::generate_n (job_ids.begin(), n_jobs, utils::random_peer_name);

  sdpa::daemon::WorkerManager _worker_manager;
  sdpa::daemon::CoallocationScheduler
    _scheduler ( [&test_transfer_cost, &_computational_cost] (const sdpa::job_id_t&)
                 {
                   return Requirements_and_preferences
                     ( {}
                     , we::type::schedule_data (n_req_workers)
                     , test_transfer_cost
                     , _computational_cost
                     , 0
                     , {}
                     );
                 }
               , _worker_manager
               );

  for (int i=0; i<n_workers;i++)
  {
    _worker_manager.addWorker (worker_ids[i], {}, random_ulong(), false, host_ids[i], fhg::util::testing::random_string());
  }

  std::for_each ( job_ids.begin()
                , job_ids.end()
                , std::bind ( &sdpa::daemon::CoallocationScheduler::enqueueJob
                            , &_scheduler
                            , std::placeholders::_1
                            )
                );

  _scheduler.assignJobsToWorkers();
  auto const assignment
    ( sdpa::daemon::access_allocation_table_TESTING_ONLY (_scheduler)
    . get_current_assignment()
    );

  std::vector<double> sum_costs_assigned_jobs (n_workers, 0.0);
  const double max_job_cost ( *std::max_element (transfer_costs.begin(), transfer_costs.end())
                            + _computational_cost
                            );

  for ( const std::set<sdpa::worker_id_t>& job_assigned_workers
      : assignment | boost::adaptors::map_values
      )
  {
    for (int k=0; k<n_workers; ++k)
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

  _worker_manager.addWorker ( "worker_0"
                            , {}
                            , avail_mem
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  const sdpa::job_id_t job_id;

  add_job ( job_id
          , Requirements_and_preferences ( {}
                                         , we::type::schedule_data()
                                         , null_transfer_cost
                                         , computational_cost
                                         , avail_mem + 1
                                         , {}
                                         )
          );

  _scheduler.enqueueJob (job_id);
  _scheduler.assignJobsToWorkers();

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

  _worker_manager.addWorker ( "worker_0"
                            , {}
                            , size_0
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  _worker_manager.addWorker ( "worker_1"
                            , {}
                            , size_1
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );


  _worker_manager.addWorker ( "worker_2"
                            , {}
                            , size_0 + size_1
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  const sdpa::job_id_t job_id_0 ("job_0");
  const sdpa::job_id_t job_id_1 ("job_1");

  add_job ( job_id_0
          , Requirements_and_preferences ( {}
                                         , we::type::schedule_data()
                                         , null_transfer_cost
                                         , computational_cost
                                         , size_0
                                         , {}
                                         )
          );

  add_job ( job_id_1
          , Requirements_and_preferences ( {}
                                         , we::type::schedule_data()
                                         , null_transfer_cost
                                         , computational_cost
                                         , size_1
                                         , {}
                                         )
          );


  _scheduler.enqueueJob (job_id_0);
  _scheduler.enqueueJob (job_id_1);

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());
    BOOST_REQUIRE (assignment.count (job_id_0));
    BOOST_REQUIRE (assignment.count (job_id_1));

    BOOST_REQUIRE_EQUAL (assignment.at (job_id_0), set_0);
    BOOST_REQUIRE_EQUAL (assignment.at (job_id_1), set_1);
  }

  _scheduler.releaseReservation (job_id_0);
  _scheduler.releaseReservation (job_id_1);

  _scheduler.enqueueJob (job_id_1);
  _scheduler.enqueueJob (job_id_0);

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());
    BOOST_REQUIRE (assignment.count (job_id_0));
    BOOST_REQUIRE (assignment.count (job_id_1));

    BOOST_REQUIRE_EQUAL (assignment.at (job_id_0), set_0);
    BOOST_REQUIRE_EQUAL (assignment.at (job_id_1), set_1);
  }
}

BOOST_FIXTURE_TEST_CASE
  ( assign_job_without_requirements_and_preferences_to_worker_with_least_capabilities
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  std::string const name_worker_0 {"0" + fhg::util::testing::random_string()};
  std::string const name_worker_1 {"1" + fhg::util::testing::random_string()};
  std::string const name_capability {fhg::util::testing::random_string()};
  _worker_manager.addWorker
    ( name_worker_0
    , {}
    , random_ulong()
    , false
    , fhg::util::testing::random_string()
    , fhg::util::testing::random_string()
    );
  _worker_manager.addWorker
    ( name_worker_1
    , {sdpa::Capability (name_capability, name_worker_1)}
    , random_ulong()
    , false
    , fhg::util::testing::random_string()
    , fhg::util::testing::random_string()
    );

  sdpa::job_id_t const job_id {fhg::util::testing::random_string()};

  add_job (job_id, no_requirements_and_preferences());
  _scheduler.enqueueJob (job_id);

  _scheduler.assignJobsToWorkers();
  auto const assignment (get_current_assignment());

  BOOST_REQUIRE_EQUAL (assignment.size(), 1);
  BOOST_REQUIRE (assignment.count (job_id));

  BOOST_REQUIRE_EQUAL
    (assignment.at (job_id), std::set<sdpa::worker_id_t> {name_worker_0});
}

BOOST_FIXTURE_TEST_CASE ( assign_job_to_the_matching_worker_with_less_capabilities_when_same_costs
                        , fixture_scheduler_and_requirements_and_preferences
                        )
{
  std::set<sdpa::worker_id_t> set_0 {"worker_0"};

  _worker_manager.addWorker ( "worker_0"
                            , {sdpa::Capability("A", "worker_0")}
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  _worker_manager.addWorker ( "worker_1"
                            , { sdpa::Capability("A", "worker_1")
                              , sdpa::Capability("B", "worker_1")
                              }
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  _worker_manager.addWorker ( "worker_2"
                            , { sdpa::Capability("A", "worker_2")
                              , sdpa::Capability("B", "worker_2")
                              , sdpa::Capability("C", "worker_2")
                              }
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  const sdpa::job_id_t job_id ("job_0");

  add_job (job_id, require ("A"));
  _scheduler.enqueueJob (job_id);

  _scheduler.assignJobsToWorkers();
  auto const assignment (get_current_assignment());

  BOOST_REQUIRE (!assignment.empty());
  BOOST_REQUIRE (assignment.count (job_id));

  BOOST_REQUIRE_EQUAL (assignment.at (job_id), set_0);
}

BOOST_FIXTURE_TEST_CASE ( assign_to_the_same_worker_if_the_total_cost_is_lower
                        , fixture_scheduler_and_requirements_and_preferences
                        )
{
  std::string const name_worker_0 {"worker_0_" + fhg::util::testing::random_string()};
  std::string const name_worker_1 {"worker_1_" + fhg::util::testing::random_string()};
  std::string const name_node_0 {"node_0_" + fhg::util::testing::random_string()};
  std::string const name_node_1 {"node_1_" + fhg::util::testing::random_string()};

  std::set<sdpa::worker_id_t> const expected_assignment {name_worker_1};

  _worker_manager.addWorker ( name_worker_0
                            , {}
                            , 199
                            , false
                            , name_node_0
                            , fhg::util::testing::random_string()
                            );

  _worker_manager.addWorker ( name_worker_1
                            , {}
                            , 200
                            , false
                            , name_node_1
                            , fhg::util::testing::random_string()
                            );

  sdpa::job_id_t const job_id_0 (fhg::util::testing::random_string());
  sdpa::job_id_t const job_id_1 (fhg::util::testing::random_string());

  std::function<double (std::string const&)> const
    test_transfer_cost ( [&name_node_0, &name_node_1](const std::string& host) -> double
                         {
                           if (host == name_node_0)
                             return 1000.0;
                           if (host == name_node_1)
                             return 1.0;
                           throw std::runtime_error ("Unexpected host argument in test transfer cost function");
                         }
                       );

  add_job ( job_id_0
          , Requirements_and_preferences ( {}
                                         , we::type::schedule_data()
                                         , test_transfer_cost
                                         , 1.0
                                         , 100
                                         , {}
                                         )
          );

  add_job ( job_id_1
          , Requirements_and_preferences ( {}
                                         , we::type::schedule_data()
                                         , test_transfer_cost
                                         , 1.0
                                         , 200
                                         , {}
                                         )
          );


  _scheduler.enqueueJob (job_id_0);
  _scheduler.enqueueJob (job_id_1);
  _scheduler.assignJobsToWorkers();
  auto const assignment (get_current_assignment());

  BOOST_REQUIRE (!assignment.empty());
  BOOST_REQUIRE (assignment.count (job_id_0));
  BOOST_REQUIRE (assignment.count (job_id_1));

  BOOST_REQUIRE_EQUAL (assignment.at (job_id_0), expected_assignment);
  BOOST_REQUIRE_EQUAL (assignment.at (job_id_1), expected_assignment);
}

BOOST_FIXTURE_TEST_CASE ( work_stealing
                        , fixture_scheduler_and_requirements_and_preferences
                        )
{
  std::set<sdpa::worker_id_t> set_0 {"worker_0"};

  _worker_manager.addWorker ( "worker_0"
                            , {sdpa::Capability("A", "worker_0")}
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  _worker_manager.addWorker ( "worker_1"
                            , { sdpa::Capability("A", "worker_1")}
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  const sdpa::job_id_t job_0 ("job_0");
  const sdpa::job_id_t job_1 ("job_1");
  const sdpa::job_id_t job_2 ("job_2");

  add_job (job_0, require ("A"));
  _scheduler.enqueueJob (job_0);

  add_job (job_1, require ("A"));
  _scheduler.enqueueJob (job_1);

  add_job (job_2, require ("A"));
  _scheduler.enqueueJob (job_2);

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());
    BOOST_REQUIRE (assignment.count (job_0));
    BOOST_REQUIRE (assignment.count (job_1));
    BOOST_REQUIRE (assignment.count (job_2));
  }

  _worker_manager.addWorker ( "worker_2"
                            , { sdpa::Capability("A", "worker_2")}
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  _scheduler.assignJobsToWorkers();

  {
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());

    BOOST_REQUIRE ( assignment.at (job_0) != std::set<sdpa::worker_id_t>({"worker_2"})
                 && assignment.at (job_1) != std::set<sdpa::worker_id_t>({"worker_2"})
                 && assignment.at (job_2) != std::set<sdpa::worker_id_t>({"worker_2"})
                  );
  }
  _scheduler.steal_work();

  {
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());
    BOOST_REQUIRE (assignment.count (job_0));
    BOOST_REQUIRE (assignment.count (job_1));
    BOOST_REQUIRE (assignment.count (job_2));

    BOOST_REQUIRE (  assignment.at (job_0) == std::set<sdpa::worker_id_t>({"worker_2"})
                  || assignment.at (job_1) == std::set<sdpa::worker_id_t>({"worker_2"})
                  || assignment.at (job_2) == std::set<sdpa::worker_id_t>({"worker_2"})
                  );

    BOOST_REQUIRE (assignment.at (job_0) != assignment.at (job_1));
    BOOST_REQUIRE (assignment.at (job_0) != assignment.at (job_2));
    BOOST_REQUIRE (assignment.at (job_1) != assignment.at (job_2));
  }
}

BOOST_FIXTURE_TEST_CASE ( stealing_from_worker_does_not_free_it
                        , fixture_scheduler_and_requirements_and_preferences
                        )
{
  _worker_manager.addWorker ( "worker_0"
                            , {sdpa::Capability("A", "worker_0")}
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  const sdpa::job_id_t job_0 ("job_0");
  const sdpa::job_id_t job_1 ("job_1");
  const sdpa::job_id_t job_2 ("job_2");

  add_job (job_0, require ("A"));
  _scheduler.enqueueJob (job_0);

  add_job (job_1, require ("A"));
  _scheduler.enqueueJob (job_1);

  add_job (job_2, require ("A"));
  _scheduler.enqueueJob (job_2);

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());
    BOOST_REQUIRE (assignment.count (job_0));
    BOOST_REQUIRE (assignment.count (job_1));
    BOOST_REQUIRE (assignment.count (job_2));

    // the worker 0 is submitted one of the assigned jobs
    std::set<sdpa::job_id_t> jobs_started (_scheduler.start_pending_jobs (serve_job));

    BOOST_REQUIRE ( jobs_started.count (job_0)
                 || jobs_started.count (job_1)
                 || jobs_started.count (job_2)
                  );
  }

  _worker_manager.addWorker ( "worker_1"
                            , { sdpa::Capability("A", "worker_1")}
                            , random_ulong()
                            , false
                            , fhg::util::testing::random_string()
                            , fhg::util::testing::random_string()
                            );

  {
    _scheduler.assignJobsToWorkers();
    auto const assignment (get_current_assignment());

    BOOST_REQUIRE (!assignment.empty());

    BOOST_REQUIRE (assignment.count (job_0));
    BOOST_REQUIRE (assignment.count (job_1));
    BOOST_REQUIRE (assignment.count (job_2));

    BOOST_REQUIRE (  assignment.at (job_0) == std::set<sdpa::worker_id_t>({"worker_0"})
                  || assignment.at (job_1) == std::set<sdpa::worker_id_t>({"worker_0"})
                  || assignment.at (job_2) == std::set<sdpa::worker_id_t>({"worker_0"})
                  );

    BOOST_REQUIRE ( assignment.at (job_0) != std::set<sdpa::worker_id_t>({"worker_1"})
                 && assignment.at (job_1) != std::set<sdpa::worker_id_t>({"worker_1"})
                 && assignment.at (job_2) != std::set<sdpa::worker_id_t>({"worker_1"})
                  );
  }

  _scheduler.steal_work();

  {
    auto const assignment (get_current_assignment());
    BOOST_REQUIRE (!assignment.empty());

    // the worker 1 is assigned a job that was stolen from the worker 0
    BOOST_REQUIRE (  assignment.at (job_0) == std::set<sdpa::worker_id_t>({"worker_1"})
                  || assignment.at (job_1) == std::set<sdpa::worker_id_t>({"worker_1"})
                  || assignment.at (job_2) == std::set<sdpa::worker_id_t>({"worker_1"})
                  );

    std::set<sdpa::job_id_t> jobs_started (_scheduler.start_pending_jobs (serve_job));

    // only the worker 1 should get a job started, as worker 0 is still reserved
    // (until the submitted job finishes)
    BOOST_REQUIRE_EQUAL (jobs_started.size(), 1);
  }
}

struct fixture_add_new_workers
{
  fixture_add_new_workers()
    : _worker_manager()
    , _scheduler
       ( std::bind ( &fixture_add_new_workers::requirements_and_preferences
                   , this
                   , std::placeholders::_1
                   )
       , _worker_manager
       )
    , _access_allocation_table (_scheduler)
  {}

  sdpa::daemon::WorkerManager _worker_manager;
  sdpa::daemon::CoallocationScheduler _scheduler;
  sdpa::daemon::access_allocation_table_TESTING_ONLY _access_allocation_table;

  std::map<sdpa::job_id_t, std::set<sdpa::worker_id_t>> get_current_assignment() const
  {
    return _access_allocation_table.get_current_assignment();
  }

  sdpa::daemon::WorkerSet const workers (sdpa::job_id_t const& job) const
  {
    return _access_allocation_table.workers (job);
  }

  sdpa::daemon::Implementation const implementation (sdpa::job_id_t const& job) const
  {
    return _access_allocation_table.implementation (job);
  }

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
      sdpa::capabilities_set_t cpbset;
      for (std::string const& capability_name : cpbnames)
      {
        cpbset.emplace (capability_name, worker);
      }

      _worker_manager.addWorker ( worker
                                , cpbset
                                , random_ulong()
                                , false
                                , fhg::util::testing::random_string()
                                , fhg::util::testing::random_string()
                                );
      request_scheduling();
    }

    return new_workers;
  }

  void add_new_jobs
    ( std::vector<sdpa::job_id_t>& a
    , boost::optional<std::string> reqname
    , unsigned int n
    )
  {
    const unsigned int N (a.size());
    unsigned int j (N);
    a.resize (N + n);
    std::generate_n ( a.begin() + N
                    , n
                    , [&j]
                      {return "job_" + std::to_string (j++);}
                    );

    BOOST_REQUIRE_EQUAL (a.size(), N + n);

    for_each ( a.begin() + N
             , a.end()
             , [this, &reqname] (sdpa::job_id_t job)
               {
                 add_job (job, reqname ? require (reqname.get())
                                       : no_requirements_and_preferences()
                         );
                 _scheduler.enqueueJob (job);
                 request_scheduling();
               }
             );
  }

  void request_scheduling()
  {
    _scheduler.assignJobsToWorkers();
    _scheduler.steal_work();
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
        : assignment | boost::adaptors::map_values
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
  ( add_new_workers_and_steal_work
  , fixture_add_new_workers
  )
{
  const unsigned int n (20);
  const unsigned int k (10);
  const unsigned int n_capabilities (3);

  std::unordered_set<std::string> capabilities;
  for (unsigned int i (0); i<n_capabilities; i++)
  {
    capabilities.emplace (fhg::util::testing::random_string());
  }

  const std::vector<sdpa::worker_id_t> initial_workers
    (add_new_workers (capabilities, n));

  std::vector<sdpa::job_id_t> jobs;
  add_new_jobs (jobs, boost::none, 2*n);

  BOOST_REQUIRE_EQUAL (get_workers_with_assigned_jobs (jobs).size(), n);

  add_new_jobs (jobs, boost::none, 2*k);

  auto const old_assignment (get_current_assignment());

  const std::vector<sdpa::worker_id_t> new_workers
    (add_new_workers (capabilities, k));

  BOOST_REQUIRE_EQUAL (get_workers_with_assigned_jobs (jobs).size(), n + k);

  check_work_stealing (initial_workers, new_workers, old_assignment, jobs.size());
}

BOOST_FIXTURE_TEST_CASE
  ( add_new_workers_with_capabilities_and_steal_work_repeatedly
  , fixture_add_new_workers
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

  add_new_workers ({"LOAD"}, n_load_workers);
  add_new_workers ({"REDUCE"}, n_reduce_workers);
  std::vector<sdpa::worker_id_t> initial_calc_workers
    (add_new_workers ({"CALC"}, n_calc_workers));

  add_new_jobs (jobs, std::string ("LOAD"), n_load_jobs);
  add_new_jobs (jobs, std::string ("CALC"), n_calc_jobs);
  add_new_jobs (jobs, std::string ("REDUCE"), n_reduce_jobs);

  BOOST_REQUIRE_EQUAL ( get_workers_with_assigned_jobs (jobs).size()
                      , n_load_workers
                      + n_calc_workers
                      + n_reduce_workers
                      );

  {
    // add a new LOAD worker
    const std::vector<sdpa::worker_id_t> new_load_workers
      (add_new_workers ({"LOAD"}, 1));

    // this last added worker shout get nothing as there is no job new generated,
    // all the other jobs were already assigned and there is nothing to
    // steal as all the LOAD workers have exactly one job assigned
    BOOST_REQUIRE_EQUAL
      (n_jobs_assigned_to_worker (new_load_workers.front(), get_current_assignment()), 0);
  }

  {
    // add a new REDUCE worker
    const std::vector<sdpa::worker_id_t> new_reduce_workers
      (add_new_workers ({"REDUCE"}, 1));

    // this last added worker shout get nothing as there is no job new generated,
    // all the other jobs were already assigned and there is nothing to
    // steal as all the REDUCE workers have exactly one job assigned
    BOOST_REQUIRE_EQUAL
      (n_jobs_assigned_to_worker (new_reduce_workers.front(), get_current_assignment()), 0);
   }

  {
    auto const old_assignment (get_current_assignment());

    // add new n_calc_workers CALC workers
    const std::vector<sdpa::worker_id_t> new_calc_workers
      (add_new_workers ({"CALC"}, n_calc_workers));

    BOOST_REQUIRE_EQUAL (new_calc_workers.size(), n_calc_workers);

    auto const assignment (get_current_assignment());

    // all CALC workers should get a job stolen from one of the  n_calc_workers
    // workers previously added (no new job was generated)
    check_work_stealing ( initial_calc_workers
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
    auto const old_assignment (get_current_assignment());

    // add new n_calc_workers CALC workers
    const std::vector<sdpa::worker_id_t> new_calc_workers
      (add_new_workers ({"CALC"}, n_calc_workers));

    BOOST_REQUIRE_EQUAL (new_calc_workers.size(), n_calc_workers);

    auto const assignment (get_current_assignment());

    // all CALC workers should get a job stolen from one of the  n_calc_workers
    // workers previously added (no new job was generated)
    check_work_stealing ( initial_calc_workers
                        , new_calc_workers
                        , old_assignment
                        , n_calc_jobs
                        );
  }
}

BOOST_FIXTURE_TEST_CASE
  ( steal_work_after_finishing_assigned_job
  , fixture_add_new_workers
  )
{
  const unsigned int n_workers (2);
  constexpr unsigned int n_jobs (n_workers + 1);
  const unsigned int n_capabilities (3);

  std::unordered_set<std::string> capabilities;
  for (unsigned int i (0); i<n_capabilities; i++)
  {
    capabilities.emplace (fhg::util::testing::random_string());
  }

  const std::vector<sdpa::worker_id_t> workers
    (add_new_workers (capabilities, n_workers));

  std::vector<sdpa::job_id_t> jobs;
  add_new_jobs (jobs, boost::none, n_jobs);

  BOOST_REQUIRE_EQUAL (get_workers_with_assigned_jobs (jobs).size(), n_workers);

  auto const old_assignment (get_current_assignment());

  sdpa::worker_id_t worker_with_2_jobs;
  sdpa::worker_id_t worker_with_1_job;

  for (const sdpa::worker_id_t& worker : workers)
  {
    const unsigned int n_assigned_jobs
      (n_jobs_assigned_to_worker (worker, old_assignment));
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
    (get_jobs_assigned_to_worker (worker_with_1_job, old_assignment));
  BOOST_REQUIRE_EQUAL (worker_jobs.size(), 1);

  _worker_manager.submit_and_serve_if_can_start_job_INDICATES_A_RACE
    ( *worker_jobs.cbegin()
    , {worker_with_1_job}
    , boost::none
    , [] ( sdpa::daemon::WorkerSet const&
         , sdpa::daemon::Implementation const&
         , sdpa::job_id_t const&
         )
      {}
    );

  _scheduler.releaseReservation (*worker_jobs.begin());

  request_scheduling();

  auto const assignment (get_current_assignment());

  // the worker which just finished the job should steal from the worker with 2 jobs
  // in the end, all workers should have exactly one job assigned
  BOOST_REQUIRE_EQUAL (n_jobs_assigned_to_worker (worker_with_2_jobs, assignment), 1);
  BOOST_REQUIRE_EQUAL (n_jobs_assigned_to_worker (worker_with_1_job, assignment), 1);
}

BOOST_FIXTURE_TEST_CASE
  ( cannot_steal_from_a_different_equivalence_class
  , fixture_add_new_workers
  )
{
  const unsigned int n_workers (2);
  constexpr unsigned int n_jobs (n_workers + 1);

  const sdpa::worker_id_t worker_0
    (add_new_workers ({"A", "B"}, 1).at (0));

  const sdpa::worker_id_t worker_1
    (add_new_workers ({"A", "C"}, 1).at (0));

  std::vector<sdpa::job_id_t> jobs;
  add_new_jobs (jobs, std::string ("A"), n_jobs);

  BOOST_REQUIRE_EQUAL (get_workers_with_assigned_jobs (jobs).size(), n_workers);

  auto const old_assignment (get_current_assignment());

  const unsigned int n_assigned_jobs_worker_0
    (n_jobs_assigned_to_worker (worker_0, old_assignment));
  BOOST_REQUIRE (n_assigned_jobs_worker_0 == 1 || n_assigned_jobs_worker_0 == 2);

  const unsigned int n_assigned_jobs_worker_1
    (n_jobs_assigned_to_worker (worker_1, old_assignment));
  BOOST_REQUIRE (n_assigned_jobs_worker_1 == 1 || n_assigned_jobs_worker_1 == 2);

  const sdpa::worker_id_t worker_with_1_job
    {(n_assigned_jobs_worker_0 == 1) ? worker_0 : worker_1};
  const sdpa::worker_id_t worker_with_2_jobs
    {(n_assigned_jobs_worker_0 == 2) ? worker_0 : worker_1};

  const std::set<sdpa::job_id_t> worker_jobs
    (get_jobs_assigned_to_worker (worker_with_1_job, old_assignment));
  BOOST_REQUIRE_EQUAL (worker_jobs.size(), 1);

  _worker_manager.submit_and_serve_if_can_start_job_INDICATES_A_RACE
    ( *worker_jobs.cbegin()
    , {worker_with_1_job}
    , boost::none
    , [] ( sdpa::daemon::WorkerSet const&
         , sdpa::daemon::Implementation
         , sdpa::job_id_t const&
         )
      {}
    );

  // the worker with 1 job finishes the assigned job
  BOOST_REQUIRE (!worker_jobs.empty());
  _scheduler.releaseReservation (*worker_jobs.begin());

  request_scheduling();

  auto const assignment (get_current_assignment());

  // the worker with 1 job and which just finished the job should NOT steal from
  // the worker with 2 jobs, as they are in different equivalence classes
  BOOST_REQUIRE_EQUAL (n_jobs_assigned_to_worker (worker_with_2_jobs, assignment), 2);
  BOOST_REQUIRE_EQUAL (n_jobs_assigned_to_worker (worker_with_1_job, assignment), 0);
}

BOOST_FIXTURE_TEST_CASE
  ( the_worker_that_is_longer_idle_gets_first_the_job
  , fixture_add_new_workers
  )
{
  const unsigned int n_workers (3);
  constexpr unsigned int n_jobs (n_workers + 1);

  const std::vector<sdpa::worker_id_t> workers
    (add_new_workers ({"A"}, 3));

  std::vector<sdpa::job_id_t> jobs;
  add_new_jobs (jobs, std::string ("A"), n_jobs);

  BOOST_REQUIRE_EQUAL (get_workers_with_assigned_jobs (jobs).size(), n_workers);

  sdpa::worker_id_t worker_with_2_jobs;
  auto const assignment (get_current_assignment());

  // only one worker should have 2 jobs assigned, the others just 1
  for (sdpa::worker_id_t const& worker : workers)
  {
    const unsigned int n_worker_jobs
      (n_jobs_assigned_to_worker (worker, assignment));
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
      (get_jobs_assigned_to_worker (worker, assignment));
    BOOST_REQUIRE_EQUAL (worker_jobs.size(), 1);

    _worker_manager.submit_and_serve_if_can_start_job_INDICATES_A_RACE
      ( *worker_jobs.cbegin()
      , {worker}
      , boost::none
      , [] ( sdpa::daemon::WorkerSet const&
           , sdpa::daemon::Implementation const&
           , sdpa::job_id_t const&
           )
        {}
      );

    _scheduler.releaseReservation (*worker_jobs.begin());

    std::this_thread::sleep_for (delay);
  }

  {
    auto const assignment (get_current_assignment());

    // One worker has 2 pending jobs, 2 workers are idle, no jobs to assign are left
    BOOST_REQUIRE_EQUAL (n_jobs_assigned_to_worker (worker_with_2_jobs, assignment), 2);
    BOOST_REQUIRE_EQUAL (n_jobs_assigned_to_worker (*workers_with_1_job.begin(), assignment), 0);
    BOOST_REQUIRE_EQUAL (n_jobs_assigned_to_worker (*std::next (workers_with_1_job.begin()), assignment), 0);
  }

  request_scheduling();

  {
    auto const assignment (get_current_assignment());

    // The worker which stayed longer idle should steal the only job to steal from
    // the worker with 2 pending jobs. The worker that stayed idle for a shorter time
    // should get nothing, as stealing is not allowed by any other worker
    BOOST_REQUIRE_EQUAL (n_jobs_assigned_to_worker (worker_with_2_jobs, assignment), 1);
    BOOST_REQUIRE_EQUAL (n_jobs_assigned_to_worker (*workers_with_1_job.begin(), assignment), 1);
    BOOST_REQUIRE_EQUAL (n_jobs_assigned_to_worker (*std::next (workers_with_1_job.begin()), assignment), 0);
  }
}

BOOST_FIXTURE_TEST_CASE
  (request_arbitrary_number_of_workers, fixture_add_new_workers)
{
  unsigned int const n_workers (1000);
  unsigned int const n_max_workers_per_job (10);

  std::vector<sdpa::worker_id_t> const workers
    (add_new_workers ({"A"}, n_workers));

  unsigned int n_assigned_workers (0);
  unsigned int k (0);
  while (n_assigned_workers < n_workers)
  {
    sdpa::job_id_t const job ("job_" + std::to_string (k));

    unsigned int const n_req_workers
      {fhg::util::testing::random_integral<unsigned int>() % n_max_workers_per_job + 1};

    add_job (job, require ("A", n_req_workers));

    _scheduler.enqueueJob (job);
    request_scheduling();

    auto const assignment (get_current_assignment());

    BOOST_REQUIRE_EQUAL (assignment.at (job).size(), n_req_workers);

    n_assigned_workers += n_req_workers;
    k++;
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
    sdpa::job_id_t const job ("job_" + std::to_string (k));

    unsigned int const n_req_workers
      { fhg::util::testing::random_integral<unsigned int>()
      % n_max_workers_per_job
      + 1
      };

    add_job (job, require ("A", n_req_workers));
    _scheduler.enqueueJob (job);
  }

  _scheduler.assignJobsToWorkers();
  _scheduler.steal_work();

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
    std::set<sdpa::job_id_t> const jobs_started
      (_scheduler.start_pending_jobs
        ( [this, &started_jobs]
             ( sdpa::daemon::WorkerSet const& assigned_workers
             , sdpa::daemon::Implementation const& implementation
             , sdpa::job_id_t const& job
             )
          {
            BOOST_REQUIRE_EQUAL
              ( this->workers (job).size()
              , _requirements_and_preferences.at (job).numWorkers()
              );

            BOOST_REQUIRE_EQUAL (this->workers (job), assigned_workers);
            BOOST_REQUIRE_EQUAL (this->implementation (job), implementation);

            ++started_jobs;
          }
        )
      );

    running_jobs.insert (jobs_started.begin(), jobs_started.end());
    if (!running_jobs.empty())
    {

      long unsigned int const n_finishing_jobs
        { fhg::util::testing::random_integral<long unsigned int>()
        % running_jobs.size()
        + 1
        };

      // finish an arbitrary number of running jobs
      for (unsigned int k = 0; k < n_finishing_jobs; k++)
      {
        _scheduler.releaseReservation (*running_jobs.begin());
        running_jobs.erase (running_jobs.begin());
      }
    }

    _scheduler.assignJobsToWorkers();
    _scheduler.steal_work();
    assignment = get_current_assignment();
  }

  BOOST_REQUIRE_EQUAL (started_jobs, n_jobs);
}

BOOST_FIXTURE_TEST_CASE
  ( assign_jobs_respecting_preferences
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  fhg::util::testing::unique_random<sdpa::worker_id_t> worker_name_pool;
  fhg::util::testing::unique_random<std::string> capability_pool;
  fhg::util::testing::unique_random<sdpa::job_id_t> job_name_pool;

  std::string const common_capability (capability_pool());

  Preferences const preferences
    { capability_pool()
    , capability_pool()
    , capability_pool()
    };

  sdpa::worker_id_t const worker_0 (worker_name_pool());
  auto const first_pref (preferences.begin());

  _worker_manager.addWorker
    ( worker_0
    , { sdpa::Capability (common_capability, worker_0)
      , sdpa::Capability (*first_pref, worker_0)
      }
    , random_ulong()
    , false
    , fhg::util::testing::random_identifier_without_leading_underscore()
    , fhg::util::testing::random_identifier_without_leading_underscore()
    );

  sdpa::worker_id_t const worker_1 (worker_name_pool());
  _worker_manager.addWorker
    ( worker_1
    , { sdpa::Capability (common_capability, worker_1)
      , sdpa::Capability (*std::next (first_pref, 1), worker_1)
      }
    , random_ulong()
    , false
    , fhg::util::testing::random_identifier_without_leading_underscore()
    , fhg::util::testing::random_identifier_without_leading_underscore()
    );

  sdpa::worker_id_t const worker_2 (worker_name_pool());
  _worker_manager.addWorker
    ( worker_2
    , { sdpa::Capability (common_capability, worker_2)
      , sdpa::Capability (*std::next (first_pref, 2), worker_2)
      }
    , random_ulong()
    , false
    , fhg::util::testing::random_identifier_without_leading_underscore()
    , fhg::util::testing::random_identifier_without_leading_underscore()
    );

  sdpa::job_id_t const job0 (job_name_pool());
  add_job (job0, require (common_capability, preferences));
  _scheduler.enqueueJob (job0);
  _scheduler.assignJobsToWorkers();
  require_worker_and_implementation (job0, worker_0, *first_pref);

  sdpa::job_id_t const job1 (job_name_pool());
  add_job (job1, require (common_capability, preferences));
  _scheduler.enqueueJob (job1);
  _scheduler.assignJobsToWorkers();
  require_worker_and_implementation (job1, worker_1, *std::next (first_pref, 1));

  sdpa::job_id_t const job2 (job_name_pool());
  add_job (job2, require (common_capability, preferences));
  _scheduler.enqueueJob (job2);
  _scheduler.assignJobsToWorkers();
  require_worker_and_implementation (job2, worker_2, *std::next (first_pref, 2));
}

BOOST_FIXTURE_TEST_CASE
  ( coallocation_with_multiple_implementations_is_forbidden
  , fixture_add_new_workers
  )
{
  fhg::util::testing::unique_random<std::string> capability_pool;
  fhg::util::testing::unique_random<sdpa::job_id_t> job_name_pool;

  std::string const common_capability (capability_pool());

  std::string preference (capability_pool());

  unsigned int const num_workers
    (2 + fhg::util::testing::random_integral<unsigned int>() % 3);

  std::vector<sdpa::worker_id_t> const workers
    (add_new_workers ( {common_capability, preference}
                     , num_workers
                     )
    );

  sdpa::job_id_t const job (job_name_pool());

  add_job
    ( job
    , require ( common_capability
              , num_workers
              , {preference, capability_pool(), capability_pool()}
              )
    );

  _scheduler.enqueueJob (job);
  fhg::util::testing::require_exception
     ( [this] { _scheduler.assignJobsToWorkers(); }
     ,  std::runtime_error
         ("Coallocation with preferences is forbidden!")
     );

}

BOOST_FIXTURE_TEST_CASE
  ( check_worker_is_served_the_corresponding_implementation
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  fhg::util::testing::unique_random<std::string> capability_pool;
  fhg::util::testing::unique_random<sdpa::job_id_t> job_name_pool;

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
    ( fhg::util::testing::random_integral<std::size_t>()
    % preferences.size()
    );
  std::string const preference
    ( (preference_id == 0)
    ? *first_pref
    : *std::next (first_pref, preference_id)
    );

  _worker_manager.addWorker
    ( worker
    , { sdpa::Capability (capability, worker)
      , sdpa::Capability (preference, worker)
      }
    , random_ulong()
    , false
    , fhg::util::testing::random_identifier_without_leading_underscore()
    , fhg::util::testing::random_identifier_without_leading_underscore()
    );

  sdpa::job_id_t const job (job_name_pool());
  add_job (job, require (capability, preferences));

  _scheduler.enqueueJob (job);
  _scheduler.assignJobsToWorkers();

  require_worker_and_implementation (job, worker, preference);

  _scheduler.start_pending_jobs
    ( [this]
      ( sdpa::daemon::WorkerSet const& assigned_workers
      , sdpa::daemon::Implementation const& implementation
      , sdpa::job_id_t const& job
      )
      {
        BOOST_REQUIRE_EQUAL
          (_requirements_and_preferences.at (job).numWorkers(), 1);

        BOOST_REQUIRE_EQUAL (this->workers (job), assigned_workers);
        BOOST_REQUIRE_EQUAL (this->implementation (job), implementation);
      }
    );
}

BOOST_FIXTURE_TEST_CASE
  ( tasks_preferring_cpus_are_assigned_cpu_workers_first
  , fixture_add_new_workers
  )
{
  const std::string CPU ("CPU");
  const std::string GPU ("GPU");

  fhg::util::testing::unique_random<std::string> capability_pool;
  fhg::util::testing::unique_random<sdpa::job_id_t> job_name_pool;

  std::string const common_capability (capability_pool());

  unsigned int const num_cpu_workers
    (10 + fhg::util::testing::random_integral<unsigned int>() % 10);

  std::vector<sdpa::worker_id_t> const cpu_workers
    (add_new_workers ( {common_capability, CPU}
                     , num_cpu_workers
                     )
    );

  std::set<sdpa::worker_id_t> expected_cpu_workers
    (cpu_workers.begin(), cpu_workers.end());

  unsigned int const num_cpu_gpu_workers
    (10 + fhg::util::testing::random_integral<unsigned int>() % 10);

  std::vector<sdpa::worker_id_t> const cpu_gpu_workers
    (add_new_workers ( {common_capability, CPU, GPU}
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
    sdpa::job_id_t const job (job_name_pool());

    add_job (job, require (common_capability, {target}));

    _scheduler.enqueueJob (job);
    request_scheduling();

    require_worker_and_implementation
      ( job
      , target == CPU ? expected_cpu_workers : expected_cpu_gpu_workers
      , target
      );
  }

  BOOST_REQUIRE (expected_cpu_workers.empty());
  BOOST_REQUIRE (expected_cpu_gpu_workers.empty());
}

BOOST_FIXTURE_TEST_CASE
  (random_workers_are_assigned_valid_implementations, fixture_add_new_workers)
{
  fhg::util::testing::unique_random<sdpa::job_id_t> job_name_pool;
  fhg::util::testing::unique_random<std::string> capability_pool;

  std::string const common_capability (capability_pool());

  Preferences preferences;

  unsigned int total_num_workers (0);
  unsigned int const num_preferences
    (3 + fhg::util::testing::random_integral<unsigned int>() % 10);

  std::map<std::string, std::set<sdpa::worker_id_t>> workers_by_preference;

  for (unsigned int i (0); i < num_preferences; ++i)
  {
    std::string const preference (capability_pool());
    preferences.emplace_back (preference);

    unsigned int const num_workers
      (100 + fhg::util::testing::random_integral<unsigned int>() % 100);
    total_num_workers += num_workers;

    std::vector<sdpa::worker_id_t> const workers
      (add_new_workers ( {common_capability, preference}
                       , num_workers
                       )
      );

    workers_by_preference.emplace
      (preference, std::set<sdpa::worker_id_t> (workers.begin(), workers.end()));
  }

  unsigned int const num_tasks
    ( 2*total_num_workers
    + fhg::util::testing::random_integral<unsigned int>() % total_num_workers
    );

  for (unsigned int i {0}; i < num_tasks; i++)
  {
    sdpa::job_id_t const task (job_name_pool());
    add_job (task, require (common_capability, preferences));

    _scheduler.enqueueJob (task);
    request_scheduling();

    auto const assignment (get_current_assignment());
    BOOST_REQUIRE (assignment.count (task));

    auto const assigned_implementation (*implementation (task));
    BOOST_REQUIRE (implementation (task));
    BOOST_REQUIRE
      ( std::find (preferences.begin(), preferences.end(), assigned_implementation)
      != preferences.end()
      );

    auto const assigned_worker (*workers (task).begin());

    BOOST_REQUIRE
      (workers_by_preference.at (assigned_implementation).count (assigned_worker));
  }
}

BOOST_FIXTURE_TEST_CASE
  ( tasks_without_preferences_are_assigned_to_workers_with_least_capabilities
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  fhg::util::testing::unique_random<sdpa::job_id_t> job_name_pool;
  fhg::util::testing::unique_random<std::string> capability_pool;
  fhg::util::testing::unique_random<sdpa::worker_id_t> worker_name_pool;

  std::string const preference (capability_pool());

  sdpa::worker_id_t const worker_0 (worker_name_pool());
  _worker_manager.addWorker
    ( worker_0
    , {sdpa::Capability (preference, worker_0)}
    , random_ulong()
    , false
    , fhg::util::testing::random_identifier_without_leading_underscore()
    , fhg::util::testing::random_identifier_without_leading_underscore()
    );

  sdpa::worker_id_t const worker_1 (worker_name_pool());
  _worker_manager.addWorker
    ( worker_1
    , {}
    , random_ulong()
    , false
    , fhg::util::testing::random_identifier_without_leading_underscore()
    , fhg::util::testing::random_identifier_without_leading_underscore()
    );

  sdpa::job_id_t const job_0 (job_name_pool());
  add_job ( job_0
          , Requirements_and_preferences
              ( {}
              , we::type::schedule_data (1)
              , null_transfer_cost
              , computational_cost
              , 0
              , {}
              )
          );

  _scheduler.enqueueJob (job_0);
  _scheduler.assignJobsToWorkers();

  require_worker_and_implementation (job_0, worker_1, boost::none);

  sdpa::job_id_t const job_1 (job_name_pool());
  add_job ( job_1
          , Requirements_and_preferences
              ( {}
              , we::type::schedule_data (1)
              , null_transfer_cost
              , computational_cost
              , 0
              , {preference}
              )
          );

  _scheduler.enqueueJob (job_1);
  _scheduler.assignJobsToWorkers();

  require_worker_and_implementation (job_1, worker_0, preference);

  sdpa::job_id_t const job_2 (job_name_pool());
  add_job ( job_2
          , Requirements_and_preferences
              ( {}
              , we::type::schedule_data (1)
              , null_transfer_cost
              , computational_cost
              , 0
              , {preference}
              )
          );

   _scheduler.enqueueJob (job_2);
   _scheduler.assignJobsToWorkers();

   require_worker_and_implementation (job_2, worker_0, preference);

   sdpa::job_id_t const job_3 (job_name_pool());
   add_job ( job_3
           , Requirements_and_preferences
               ( {}
               , we::type::schedule_data (1)
               , null_transfer_cost
               , computational_cost
               , 0
               , {}
               )
           );

    _scheduler.enqueueJob (job_3);
    _scheduler.assignJobsToWorkers();

    require_worker_and_implementation (job_3, worker_1, boost::none);
}

BOOST_FIXTURE_TEST_CASE
  ( no_assignment_to_workers_with_no_capability_among_preferences_is_allowed
  , fixture_scheduler_and_requirements_and_preferences
  )
{
  fhg::util::testing::unique_random<sdpa::job_id_t> job_name_pool;
  fhg::util::testing::unique_random<std::string> capability_pool;
  fhg::util::testing::unique_random<sdpa::worker_id_t> worker_name_pool;

  sdpa::worker_id_t const worker (worker_name_pool());
  _worker_manager.addWorker
    ( worker
    , {sdpa::Capability (capability_pool(), worker)}
    , random_ulong()
    , false
    , fhg::util::testing::random_identifier_without_leading_underscore()
    , fhg::util::testing::random_identifier_without_leading_underscore()
    );

  sdpa::job_id_t const job (job_name_pool());
  add_job ( job
          , Requirements_and_preferences
              ( {}
              , we::type::schedule_data (1)
              , null_transfer_cost
              , computational_cost
              , 0
              , {capability_pool(), capability_pool(), capability_pool()}
              )
          );

  _scheduler.enqueueJob (job);
  _scheduler.assignJobsToWorkers();

  auto const assignment (get_current_assignment());
  BOOST_REQUIRE_EQUAL (assignment.count (job), 0);
}
