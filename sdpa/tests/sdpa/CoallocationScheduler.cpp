#define BOOST_TEST_MODULE TestScheduler
#include <utils.hpp>
#include <boost/test/unit_test.hpp>

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/set.hpp>
#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/random_string.hpp>
#include <util-generic/testing/random_integral.hpp>

#include <boost/iterator/transform_iterator.hpp>

#include <iostream>
#include <functional>
#include <random>

namespace
{
  std::string (&random_job_id)(void) = utils::random_peer_name;
  auto serve_job = [] (const sdpa::worker_id_list_t&, const sdpa::job_id_t&) {};

  unsigned long random_ulong()
  {
    return fhg::util::testing::random_integral<unsigned long>();
  }
}

struct fixture_scheduler_and_requirements
{
  typedef std::set<sdpa::worker_id_t> set_workers_t;
  typedef std::set<sdpa::job_id_t> set_jobs_t;

  fixture_scheduler_and_requirements()
    : _scheduler
      (std::bind (&fixture_scheduler_and_requirements::requirements, this, std::placeholders::_1))
  {}

  sdpa::daemon::CoallocationScheduler _scheduler;

  ~fixture_scheduler_and_requirements()
  {
  }

  void add_job (const sdpa::job_id_t& job_id, const job_requirements_t& reqs)
  {
    if (!_requirements.emplace (job_id, reqs).second)
    {
      throw std::runtime_error ("added job twice");
    }
  }

  job_requirements_t requirements (sdpa::job_id_t id)
  {
    return _requirements.find (id)->second;
  }

  std::map<sdpa::job_id_t, job_requirements_t> _requirements;

  unsigned long count_assigned_jobs ( sdpa::daemon::CoallocationScheduler::assignment_t assignment
                                    , const sdpa::worker_id_t& worker_id
                                    )
  {
    auto value = [](const sdpa::daemon::CoallocationScheduler::assignment_t::value_type& p)
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
};

namespace
{
  const double computational_cost = 1.0;

  job_requirements_t require (std::string name_1)
  {
    return {{we::type::requirement_t (name_1, true)}, we::type::schedule_data(), null_transfer_cost, computational_cost, 0};
  }

  job_requirements_t require (std::string name, unsigned long workers)
  {
    return {{we::type::requirement_t (name, true)}, we::type::schedule_data (workers), null_transfer_cost, computational_cost, 0};
  }

  job_requirements_t require (unsigned long workers)
  {
    return {{}, we::type::schedule_data (workers), null_transfer_cost, computational_cost, 0};
  }

  job_requirements_t no_requirements()
  {
    return {{}, we::type::schedule_data(), null_transfer_cost, computational_cost, 0};
  }
}

BOOST_FIXTURE_TEST_CASE (load_balancing, fixture_scheduler_and_requirements)
{
  _scheduler.worker_manager().addWorker ( "worker_0"
                                        , 1
                                        , {}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  _scheduler.worker_manager().addWorker ( "worker_1"
                                        , 1
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
    add_job (job_id, no_requirements());
    _scheduler.enqueueJob (job_id);
  }

  const sdpa::daemon::CoallocationScheduler::assignment_t
    assignment (_scheduler.assignJobsToWorkers());

  unsigned long const assigned_to_worker_0
    (count_assigned_jobs (assignment, "worker_0"));
  unsigned long const assigned_to_worker_1
    (count_assigned_jobs (assignment, "worker_1"));

  BOOST_REQUIRE (  (assigned_to_worker_0 <= assigned_to_worker_1 + 1UL)
                && (assigned_to_worker_1 <= assigned_to_worker_0 + 1UL)
                );
}

BOOST_FIXTURE_TEST_CASE (tesLBOneWorkerJoinsLater, fixture_scheduler_and_requirements)
{
  _scheduler.worker_manager().addWorker ( "worker_0"
                                        , 1
                                        , {}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  add_job ("job_0", no_requirements());
  add_job ("job_1", no_requirements());

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");

  const sdpa::daemon::CoallocationScheduler::assignment_t
    assignment (_scheduler.assignJobsToWorkers());

  BOOST_REQUIRE_EQUAL (count_assigned_jobs (assignment, "worker_0"), 2);

  _scheduler.worker_manager().addWorker ( "worker_1"
                                        , 1
                                        , {}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  _scheduler.reschedule_pending_jobs_matching_worker ("worker_1");

  const sdpa::daemon::CoallocationScheduler::assignment_t
    new_assignment (_scheduler.assignJobsToWorkers());

  BOOST_REQUIRE_EQUAL (count_assigned_jobs (new_assignment, "worker_0"), 1);
  BOOST_REQUIRE_EQUAL (count_assigned_jobs (new_assignment, "worker_1"), 1);
}


BOOST_FIXTURE_TEST_CASE (tesLBOneWorkerGainsCpbLater, fixture_scheduler_and_requirements)
{
  _scheduler.worker_manager().addWorker ( "worker_0"
                                        , 1
                                        , {sdpa::capability_t ("C", "worker_0")}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  _scheduler.worker_manager().addWorker ( "worker_1"
                                        , 1
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

  const sdpa::daemon::CoallocationScheduler::assignment_t
    assignment (_scheduler.assignJobsToWorkers());

  BOOST_REQUIRE_EQUAL (count_assigned_jobs (assignment, "worker_0"), 2);

  _scheduler.worker_manager().add_worker_capabilities ("worker_1", {sdpa::capability_t ("C", "worker_1")});
  _scheduler.reschedule_pending_jobs_matching_worker ("worker_1");

  const sdpa::daemon::CoallocationScheduler::assignment_t
    new_assignment (_scheduler.assignJobsToWorkers());

  BOOST_REQUIRE_EQUAL (count_assigned_jobs (new_assignment, "worker_0"), 1);
  BOOST_REQUIRE_EQUAL (count_assigned_jobs (new_assignment, "worker_1"), 1);
}

BOOST_FIXTURE_TEST_CASE (testCoallocSched, fixture_scheduler_and_requirements)
{
  _scheduler.worker_manager().addWorker ( "A0"
                                        , 1
                                        , {sdpa::capability_t ("A", "A0")}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  _scheduler.worker_manager().addWorker ( "B0"
                                        , 1
                                        , {sdpa::capability_t ("B", "B0")}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  _scheduler.worker_manager().addWorker ( "A1"
                                        , 1
                                        , {sdpa::capability_t ("A", "A1")}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  _scheduler.worker_manager().addWorker ( "B1"
                                        , 1
                                        , {sdpa::capability_t ("B", "B1")}
                                        , random_ulong()
                                        , false, fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  add_job ("2A", require ("A", 2));
  add_job ("2B", require ("B", 2));

  _scheduler.enqueueJob ("2A");
  _scheduler.enqueueJob ("2B");

  const sdpa::daemon::CoallocationScheduler::assignment_t
    assignment (_scheduler.assignJobsToWorkers());

  BOOST_REQUIRE_EQUAL (assignment.at ("2A"), set_workers_t ({"A0", "A1"}));
  BOOST_REQUIRE_EQUAL (assignment.at ("2B"), set_workers_t ({"B0", "B1"}));

  add_job ("1A", require ("A", 1));

  _scheduler.enqueueJob ("1A");

  const sdpa::daemon::CoallocationScheduler::assignment_t
    new_assignment (_scheduler.assignJobsToWorkers());

  BOOST_REQUIRE ( new_assignment.at ("1A") == set_workers_t ({"A0"})
               || new_assignment.at ("1A") == set_workers_t ({"A1"})
                );
}

BOOST_FIXTURE_TEST_CASE (tesLBStopRestartWorker, fixture_scheduler_and_requirements)
{
  _scheduler.worker_manager().addWorker ( "worker_0"
                                        , 1
                                        , {}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  _scheduler.worker_manager().addWorker ( "worker_1"
                                        , 1
                                        , {}
                                        , random_ulong()
                                        , false, fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  add_job ("job_0", no_requirements());
  add_job ("job_1", no_requirements());

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");

  const sdpa::daemon::CoallocationScheduler::assignment_t
    assignment (_scheduler.assignJobsToWorkers());

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
  _scheduler.worker_manager().deleteWorker ("worker_0");
  _scheduler.enqueueJob (job_assigned_to_worker_0);

  _scheduler.worker_manager().addWorker ( "worker_0"
                                        , 1
                                        , {}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  _scheduler.reschedule_pending_jobs_matching_worker ("worker_0");

  const sdpa::daemon::CoallocationScheduler::assignment_t
    new_assignment (_scheduler.assignJobsToWorkers());

  BOOST_REQUIRE_EQUAL ( assignment.at (job_assigned_to_worker_0)
                      , set_workers_t ({"worker_0"})
                      );
}

BOOST_FIXTURE_TEST_CASE
  (not_schedulable_job_does_not_block_others, fixture_scheduler_and_requirements)
{
  _scheduler.worker_manager().addWorker ( "worker"
                                        , 1
                                        , {}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  add_job ("2", require (2));
  _scheduler.enqueueJob ("2");

  const sdpa::daemon::CoallocationScheduler::assignment_t
    assignment (_scheduler.assignJobsToWorkers());

  BOOST_REQUIRE (assignment.empty());

  add_job ("1", require (1));
  _scheduler.enqueueJob ("1");

  const sdpa::daemon::CoallocationScheduler::assignment_t
    new_assignment (_scheduler.assignJobsToWorkers());

  BOOST_REQUIRE_EQUAL ( new_assignment.at ("1")
                      , set_workers_t ({"worker"})
                      );

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({"1"})
                      );
}

BOOST_FIXTURE_TEST_CASE
  (multiple_job_submissions_no_requirements, fixture_scheduler_and_requirements)
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  _scheduler.worker_manager().addWorker ( worker_id
                                        , boost::none
                                        , {}
                                        , random_ulong()
                                        , true
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  sdpa::job_id_t const job_id_0 (random_job_id());
  add_job (job_id_0, no_requirements());
  _scheduler.enqueueJob (job_id_0);

  BOOST_REQUIRE_EQUAL ( _scheduler.assignJobsToWorkers().at (job_id_0)
                      , set_workers_t ({worker_id})
                      );

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_0})
                      );

  sdpa::job_id_t const job_id_1 (random_job_id());
  add_job (job_id_1, no_requirements());
  _scheduler.enqueueJob (job_id_1);

  BOOST_REQUIRE_EQUAL ( _scheduler.assignJobsToWorkers().at (job_id_1)
                      , set_workers_t ({worker_id})
                      );

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_1})
                      );
}

BOOST_FIXTURE_TEST_CASE ( multiple_job_submissions_with_no_children_allowed
                        , fixture_scheduler_and_requirements
                        )
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  _scheduler.worker_manager().addWorker ( worker_id
                                        , boost::none
                                        , {}
                                        , random_ulong()
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  sdpa::job_id_t const job_id_0 (random_job_id());
  add_job (job_id_0, no_requirements());
  _scheduler.enqueueJob (job_id_0);

  BOOST_REQUIRE_EQUAL ( _scheduler.assignJobsToWorkers().at (job_id_0)
                      , set_workers_t ({worker_id})
                      );

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_0})
                      );

  sdpa::job_id_t const job_id_1 (random_job_id());
  add_job (job_id_1, no_requirements());
  _scheduler.enqueueJob (job_id_1);

  BOOST_REQUIRE_EQUAL ( _scheduler.assignJobsToWorkers().at (job_id_1)
                      , set_workers_t ({worker_id})
                      );

  BOOST_REQUIRE (_scheduler.start_pending_jobs (serve_job).empty());

  _scheduler.releaseReservation (job_id_0);

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                       , set_jobs_t ({job_id_1})
                       );
}

BOOST_FIXTURE_TEST_CASE
  (multiple_worker_job_submissions_with_requirements, fixture_scheduler_and_requirements)
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  _scheduler.worker_manager().addWorker ( worker_id
                                        , boost::none
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

  BOOST_REQUIRE_EQUAL ( _scheduler.assignJobsToWorkers().at (job_id_0)
                      , set_workers_t ({worker_id})
                      );

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_0})
                      );

  sdpa::job_id_t const job_id_1 (random_job_id());
  add_job (job_id_1, require ("B"));
  _scheduler.enqueueJob (job_id_1);

  BOOST_REQUIRE_EQUAL ( _scheduler.assignJobsToWorkers().at (job_id_1)
                      , set_workers_t ({worker_id})
                      );

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_1})
                      );
}

BOOST_FIXTURE_TEST_CASE ( multiple_worker_job_submissions_with_requirements_no_children_allowed
                        , fixture_scheduler_and_requirements
                        )
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  _scheduler.worker_manager().addWorker ( worker_id
                                        , boost::none
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

  BOOST_REQUIRE_EQUAL ( _scheduler.assignJobsToWorkers().at (job_id_0)
                      , set_workers_t ({worker_id})
                      );

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_0})
                      );

  sdpa::job_id_t const job_id_1 (utils::random_peer_name());
  add_job (job_id_1, require ("B"));
  _scheduler.enqueueJob (job_id_1);

  BOOST_REQUIRE_EQUAL ( _scheduler.assignJobsToWorkers().at (job_id_1)
                      , set_workers_t ({worker_id})
                      );

   BOOST_REQUIRE (_scheduler.start_pending_jobs (serve_job).empty());

  _scheduler.releaseReservation (job_id_0);

  BOOST_REQUIRE_EQUAL ( _scheduler.start_pending_jobs (serve_job)
                      , set_jobs_t ({job_id_1})
                      );
}

struct fixture_minimal_cost_assignment
{
  fixture_minimal_cost_assignment()
  : _scheduler
      ([](const sdpa::job_id_t&) {return no_requirements();})
  {
    _scheduler.worker_manager().addWorker ("worker_01", 1, {}, random_ulong(), false, "node1", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_02", 1, {}, random_ulong(), false, "node1", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_03", 1, {}, random_ulong(), false, "node1", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_04", 1, {}, random_ulong(), false, "node1", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_05", 1, {}, random_ulong(), false, "node2", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_06", 1, {}, random_ulong(), false, "node2", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_07", 1, {}, random_ulong(), false, "node2", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_08", 1, {}, random_ulong(), false, "node2", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_09", 1, {}, random_ulong(), false, "node3", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_10", 1, {}, random_ulong(), false, "node3", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_11", 1, {}, random_ulong(), false, "node3", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_12", 1, {}, random_ulong(), false, "node3", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_13", 1, {}, random_ulong(), false, "node4", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_14", 1, {}, random_ulong(), false, "node4", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_15", 1, {}, random_ulong(), false, "node4", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_16", 1, {}, random_ulong(), false, "node4", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_17", 1, {}, random_ulong(), false, "node5", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_18", 1, {}, random_ulong(), false, "node5", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_19", 1, {}, random_ulong(), false, "node5", fhg::util::testing::random_string());
    _scheduler.worker_manager().addWorker ("worker_20", 1, {}, random_ulong(), false, "node5", fhg::util::testing::random_string());
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

  void check_scheduler_finds_minimal_cost_assignement ( const std::map<std::string, double>& map_host_transfer_cost
                                                      , const sdpa::mmap_match_deg_worker_id_t& mmap_match_deg_worker
                                                      , const size_t n_req_workers
                                                      , const double min_total_cost
                                                      )
  {
    const double max_cost (max_value (map_host_transfer_cost));

    const std::function<double (std::string const&)> transfer_cost
      {[&map_host_transfer_cost, &max_cost](const std::string& host_id)
       {return map_host_transfer_cost.count (host_id) ? map_host_transfer_cost.at (host_id) : max_cost + 1;}
      };

    const std::set<sdpa::worker_id_t> set_assigned_workers
      (_scheduler.find_job_assignment_minimizing_total_cost
        (mmap_match_deg_worker, n_req_workers, transfer_cost, 1.0)
      );

    BOOST_REQUIRE_EQUAL (set_assigned_workers.size(), n_req_workers);

    std::map<sdpa::worker_id_t, double> map_worker_cost;
    std::transform ( mmap_match_deg_worker.begin()
                   , mmap_match_deg_worker.end()
                   , std::inserter (map_worker_cost, map_worker_cost.begin())
                   , [&map_host_transfer_cost] (const sdpa::mmap_match_deg_worker_id_t::value_type p)
                     {return std::make_pair ( p.second.worker_id()
                                            , map_host_transfer_cost.at(p.second.worker_host())
                                            );
                     }
                   );

    BOOST_REQUIRE_EQUAL ( min_total_cost
                        , std::accumulate ( set_assigned_workers.begin()
                                          , set_assigned_workers.end()
                                          , 0.0
                                          , [&map_worker_cost] (const double total, const sdpa::worker_id_t wid)
                                            {return total + map_worker_cost.at (wid);}
                                          )
                        );
  }

  sdpa::daemon::CoallocationScheduler _scheduler;
};

BOOST_FIXTURE_TEST_CASE ( scheduling_with_data_locality_different_matching_degs_different_costs
                        , fixture_minimal_cost_assignment
                        )
{
  // assume we have 5 nodes and the transfer cost for each is its rank
  const std::map<std::string, double> map_host_transfer_cost
    { {"node_1", 1.0}
    , {"node_2", 2.0}
    , {"node_3", 3.0}
    , {"node_4", 4.0}
    , {"node_5", 5.0}
    };

  // find an allocation minimizing the transfer costs for 5 workers
  const size_t n_req_workers (5);
  const double min_total_cost (6.0);

  // assume that we have 20 workers, i.e. 4 workers per host
  // first 4 on the "node_0", the next 4 on the "node_1" and so on
  const sdpa::mmap_match_deg_worker_id_t mmap_match_deg_worker
    { {20, {"worker_20", "node_5", random_ulong(), 0.0}}
    , {19, {"worker_19", "node_5", random_ulong(), 0.0}}
    , {18, {"worker_18", "node_5", random_ulong(), 0.0}}
    , {17, {"worker_17", "node_5", random_ulong(), 0.0}}
    , {16, {"worker_16", "node_4", random_ulong(), 0.0}}
    , {15, {"worker_15", "node_4", random_ulong(), 0.0}}
    , {14, {"worker_14", "node_4", random_ulong(), 0.0}}
    , {13, {"worker_13", "node_4", random_ulong(), 0.0}}
    , {12, {"worker_12", "node_3", random_ulong(), 0.0}}
    , {11, {"worker_11", "node_3", random_ulong(), 0.0}}
    , {10, {"worker_10", "node_3", random_ulong(), 0.0}}
    , { 9, {"worker_09", "node_3", random_ulong(), 0.0}}
    , { 8, {"worker_08", "node_2", random_ulong(), 0.0}}
    , { 7, {"worker_07", "node_2", random_ulong(), 0.0}}
    , { 6, {"worker_06", "node_2", random_ulong(), 0.0}}
    , { 5, {"worker_05", "node_2", random_ulong(), 0.0}}
    , { 4, {"worker_04", "node_1", random_ulong(), 0.0}}
    , { 3, {"worker_03", "node_1", random_ulong(), 0.0}}
    , { 2, {"worker_02", "node_1", random_ulong(), 0.0}}
    , { 1, {"worker_01", "node_1", random_ulong(), 0.0}}
    };

  check_scheduler_finds_minimal_cost_assignement ( map_host_transfer_cost
                                                 , mmap_match_deg_worker
                                                 , n_req_workers
                                                 , min_total_cost
                                                 );
}

BOOST_FIXTURE_TEST_CASE ( scheduling_with_data_locality_different_matching_degs_equal_costs
                        , fixture_minimal_cost_assignment
                        )
{
  // assume we have 5 nodes and the transfer cost is the same for all hosts
  const std::map<std::string, double> map_host_transfer_cost
    { {"node_1", 1.0}
    , {"node_2", 1.0}
    , {"node_3", 1.0}
    , {"node_4", 1.0}
    , {"node_5", 1.0}
    };

  // find an allocation minimizing the transfer costs for 5 workers
  const size_t n_req_workers (5);
  const double min_total_cost (5.0);

  // assume that we have 20 workers, i.e. 4 workers per host
  // first 4 on "node_0", the next 4 on the "node_1" and so on
  const sdpa::mmap_match_deg_worker_id_t mmap_match_deg_worker
    { {20, {"worker_20", "node_5", random_ulong(), 0.0}}
    , {19, {"worker_19", "node_5", random_ulong(), 0.0}}
    , {18, {"worker_18", "node_5", random_ulong(), 0.0}}
    , {17, {"worker_17", "node_5", random_ulong(), 0.0}}
    , {16, {"worker_16", "node_4", random_ulong(), 0.0}}
    , {15, {"worker_15", "node_4", random_ulong(), 0.0}}
    , {14, {"worker_14", "node_4", random_ulong(), 0.0}}
    , {13, {"worker_13", "node_4", random_ulong(), 0.0}}
    , {12, {"worker_12", "node_3", random_ulong(), 0.0}}
    , {11, {"worker_11", "node_3", random_ulong(), 0.0}}
    , {10, {"worker_10", "node_3", random_ulong(), 0.0}}
    , { 9, {"worker_09", "node_3", random_ulong(), 0.0}}
    , { 8, {"worker_08", "node_2", random_ulong(), 0.0}}
    , { 7, {"worker_07", "node_2", random_ulong(), 0.0}}
    , { 6, {"worker_06", "node_2", random_ulong(), 0.0}}
    , { 5, {"worker_05", "node_2", random_ulong(), 0.0}}
    , { 4, {"worker_04", "node_1", random_ulong(), 0.0}}
    , { 3, {"worker_03", "node_1", random_ulong(), 0.0}}
    , { 2, {"worker_02", "node_1", random_ulong(), 0.0}}
    , { 1, {"worker_01", "node_1", random_ulong(), 0.0}}
    };

  check_scheduler_finds_minimal_cost_assignement ( map_host_transfer_cost
                                                 , mmap_match_deg_worker
                                                 , n_req_workers
                                                 , min_total_cost
                                                 );
}

BOOST_FIXTURE_TEST_CASE ( scheduling_with_data_locality_equal_matching_degs_different_costs
                        , fixture_minimal_cost_assignment
                        )
{
  // assume we have 5 nodes and the transfer cost is different for any host
  const std::map<std::string, double> map_host_transfer_cost
    { {"node_1", 5.0}
    , {"node_2", 4.0}
    , {"node_3", 3.0}
    , {"node_4", 2.0}
    , {"node_5", 1.0}
    };

  // find an allocation minimizing the transfer costs for 5 workers
  const size_t n_req_workers (5);
  const double min_total_cost (6.0);

  // assume that we have 20 workers, i.e. 4 workers per host
  // first 4 on "node_0", the next 4 on the "node_1" and so on
  const sdpa::mmap_match_deg_worker_id_t mmap_match_deg_worker
    { {1, {"worker_20", "node_5", random_ulong(), 0.0}}
    , {1, {"worker_19", "node_5", random_ulong(), 0.0}}
    , {1, {"worker_18", "node_5", random_ulong(), 0.0}}
    , {1, {"worker_17", "node_5", random_ulong(), 0.0}}
    , {1, {"worker_16", "node_4", random_ulong(), 0.0}}
    , {1, {"worker_15", "node_4", random_ulong(), 0.0}}
    , {1, {"worker_14", "node_4", random_ulong(), 0.0}}
    , {1, {"worker_13", "node_4", random_ulong(), 0.0}}
    , {1, {"worker_12", "node_3", random_ulong(), 0.0}}
    , {1, {"worker_11", "node_3", random_ulong(), 0.0}}
    , {1, {"worker_10", "node_3", random_ulong(), 0.0}}
    , {1, {"worker_09", "node_3", random_ulong(), 0.0}}
    , {1, {"worker_08", "node_2", random_ulong(), 0.0}}
    , {1, {"worker_07", "node_2", random_ulong(), 0.0}}
    , {1, {"worker_06", "node_2", random_ulong(), 0.0}}
    , {1, {"worker_05", "node_2", random_ulong(), 0.0}}
    , {1, {"worker_04", "node_1", random_ulong(), 0.0}}
    , {1, {"worker_03", "node_1", random_ulong(), 0.0}}
    , {1, {"worker_02", "node_1", random_ulong(), 0.0}}
    , {1, {"worker_01", "node_1", random_ulong(), 0.0}}
    };

  check_scheduler_finds_minimal_cost_assignement ( map_host_transfer_cost
                                                 , mmap_match_deg_worker
                                                 , n_req_workers
                                                 , min_total_cost
                                                 );
}

BOOST_FIXTURE_TEST_CASE ( scheduling_with_data_locality_equal_matching_degs_equal_costs
                        , fixture_minimal_cost_assignment
                        )
{
  // assume we have 5 nodes and the transfer cost is the same for all hosts
  const std::map<std::string, double> map_host_transfer_cost
    { {"node_1", 1.0}
    , {"node_2", 1.0}
    , {"node_3", 1.0}
    , {"node_4", 1.0}
    , {"node_5", 1.0}
    };

  // find an allocation minimizing the transfer costs for 5 workers
  const size_t n_req_workers (5);
  const double min_total_cost (5.0);

  // assume that we have 20 workers, i.e. 4 workers per host
  // first 4 on "node_0", the next 4 on the "node_1" and so on
  const sdpa::mmap_match_deg_worker_id_t mmap_match_deg_worker
    { {1, {"worker_20", "node_5", random_ulong(), 0.0}}
    , {1, {"worker_19", "node_5", random_ulong(), 0.0}}
    , {1, {"worker_18", "node_5", random_ulong(), 0.0}}
    , {1, {"worker_17", "node_5", random_ulong(), 0.0}}
    , {1, {"worker_16", "node_4", random_ulong(), 0.0}}
    , {1, {"worker_15", "node_4", random_ulong(), 0.0}}
    , {1, {"worker_14", "node_4", random_ulong(), 0.0}}
    , {1, {"worker_13", "node_4", random_ulong(), 0.0}}
    , {1, {"worker_12", "node_3", random_ulong(), 0.0}}
    , {1, {"worker_11", "node_3", random_ulong(), 0.0}}
    , {1, {"worker_10", "node_3", random_ulong(), 0.0}}
    , {1, {"worker_09", "node_3", random_ulong(), 0.0}}
    , {1, {"worker_08", "node_2", random_ulong(), 0.0}}
    , {1, {"worker_07", "node_2", random_ulong(), 0.0}}
    , {1, {"worker_06", "node_2", random_ulong(), 0.0}}
    , {1, {"worker_05", "node_2", random_ulong(), 0.0}}
    , {1, {"worker_04", "node_1", random_ulong(), 0.0}}
    , {1, {"worker_03", "node_1", random_ulong(), 0.0}}
    , {1, {"worker_02", "node_1", random_ulong(), 0.0}}
    , {1, {"worker_01", "node_1", random_ulong(), 0.0}}
    };

  check_scheduler_finds_minimal_cost_assignement ( map_host_transfer_cost
                                                 , mmap_match_deg_worker
                                                 , n_req_workers
                                                 , min_total_cost
                                                 );
}

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
    for (const sdpa::worker_id_t worker : worker_ids)
    {
      map_costs.insert (std::make_pair (worker, dist (gen)));
    }

    return map_costs;
  }

  void serve_and_check_assignment ( const std::function<double (std::string const&)> cost
                                  , const std::vector<std::string>& worker_ids
                                  , const sdpa::worker_id_list_t& assigned_worker_ids
                                  , const sdpa::job_id_t&
                                  )
  {
    sdpa::worker_id_t assigned_worker_with_max_cost
      (*std::max_element ( assigned_worker_ids.begin()
                         , assigned_worker_ids.end()
                         , [cost](const sdpa::worker_id_t& wid_l, const sdpa::worker_id_t& wid_r)
                           { return cost (wid_l) < cost (wid_r); }
                         )
      );

    for (const sdpa::worker_id_t& wid : worker_ids)
    {
      if (std::find (assigned_worker_ids.begin(), assigned_worker_ids.end(), wid) == assigned_worker_ids.end())
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

  sdpa::daemon::CoallocationScheduler
    _scheduler (  [n_req_workers, &test_transfer_cost] (const sdpa::job_id_t&)
                  {
                    return job_requirements_t ( {}
                                              , we::type::schedule_data (n_req_workers)
                                              , test_transfer_cost
                                              , computational_cost
                                              , 0
                                              );
                  }
                );

  for (const sdpa::worker_id_t worker_id : worker_ids)
  {
    _scheduler.worker_manager().addWorker (worker_id, 1, {}, random_ulong(), false, worker_id, fhg::util::testing::random_string());
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
               )
    );

  // require the job to be scheduled
  BOOST_REQUIRE_EQUAL (_scheduler.delete_job (job_id), 0);
}

BOOST_FIXTURE_TEST_CASE ( no_coallocation_job_with_requirements_is_assigned_if_not_all_workers_are_leaves
                        , fixture_scheduler_and_requirements
                        )
{
  sdpa::worker_id_t const agent_id (utils::random_peer_name());

  _scheduler.worker_manager().addWorker ( agent_id
                                        , boost::none
                                        , {sdpa::capability_t ("A", utils::random_peer_name())}
                                        , random_ulong()
                                        , true
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  sdpa::worker_id_t const worker_id (utils::random_peer_name());
  _scheduler.worker_manager().addWorker ( worker_id
                                        , boost::none
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

BOOST_FIXTURE_TEST_CASE ( no_coallocation_job_without_requirements_is_assigned_if_not_all_workers_are_leaves
                        , fixture_scheduler_and_requirements
                        )
{
  sdpa::worker_id_t const agent_id (utils::random_peer_name());
  _scheduler.worker_manager().addWorker ( agent_id
                                        , boost::none
                                        , {}
                                        , random_ulong()
                                        , true
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  sdpa::worker_id_t const worker_id (utils::random_peer_name());
  _scheduler.worker_manager().addWorker ( worker_id
                                        , boost::none
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

  const double computational_cost (dist (rand_engine));
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

  sdpa::daemon::CoallocationScheduler
    _scheduler ( [&test_transfer_cost, &computational_cost] (const sdpa::job_id_t&)
                 {
                   return job_requirements_t ( {}
                                             , we::type::schedule_data (n_req_workers)
                                             , test_transfer_cost
                                             , computational_cost
                                             , 0
                                             );
                 }
               );

  for (int i=0; i<n_workers;i++)
  {
    _scheduler.worker_manager().addWorker (worker_ids[i], 1, {}, random_ulong(), false, host_ids[i], fhg::util::testing::random_string());
  }

  std::for_each ( job_ids.begin()
                , job_ids.end()
                , std::bind ( &sdpa::daemon::CoallocationScheduler::enqueueJob
                            , &_scheduler
                            , std::placeholders::_1
                            )
                );

  const sdpa::daemon::CoallocationScheduler::assignment_t
    assignment (_scheduler.assignJobsToWorkers());

  std::vector<double> sum_costs_assigned_jobs (n_workers, 0.0);
  const double max_job_cost ( *std::max_element (transfer_costs.begin(), transfer_costs.end())
                            + computational_cost
                            );

  for ( const std::set<sdpa::worker_id_t>& job_assigned_workers
      : assignment | boost::adaptors::map_values
      )
  {
    for (int k=0; k<n_workers; ++k)
    {
      sum_costs_assigned_jobs[k] += job_assigned_workers.count (worker_ids[k])
                                  * (transfer_costs[k] + computational_cost);
    }
  }

  BOOST_REQUIRE_LE ( std::abs ( *std::max_element (sum_costs_assigned_jobs.begin(), sum_costs_assigned_jobs.end())
                              - *std::min_element (sum_costs_assigned_jobs.begin(), sum_costs_assigned_jobs.end())
                              )
                   , max_job_cost
                   );
}

BOOST_AUTO_TEST_CASE (scheduling_bunch_of_jobs_with_re_assignment_when_new_matching_worker_appears)
{
  const unsigned int n_jobs (1000);
  const unsigned int n_req_workers (1);

  const std::vector<sdpa::worker_id_t>
    worker_ids {"worker_0", "worker_1"};

  std::uniform_real_distribution<double> dist (1.0, 10.0);
  std::random_device rand_dev;
  std::mt19937 rand_engine (rand_dev());

  const double computational_cost (dist (rand_engine));
  const double transfer_cost_host_0 (dist (rand_engine));
  const double transfer_cost_host_1 (dist (rand_engine));

  const std::function<double (std::string const&)>
    test_transfer_cost ( [&transfer_cost_host_0, &transfer_cost_host_1]
                         (const std::string& host) -> double
                         {
                           if (host == "host_0")
                             return transfer_cost_host_0;
                           if (host == "host_1")
                             return transfer_cost_host_1;
                           throw std::runtime_error ("Unexpected host argument in test transfer cost function");
                         }
                       );

  std::vector<sdpa::job_id_t> job_ids (n_jobs);
  std::generate_n (job_ids.begin(), n_jobs, utils::random_peer_name);

  sdpa::daemon::CoallocationScheduler
    _scheduler ( [n_req_workers, &test_transfer_cost, &computational_cost] (const sdpa::job_id_t&)
                 {
                   return job_requirements_t ( {}
                                             , we::type::schedule_data (n_req_workers)
                                             , test_transfer_cost
                                             , computational_cost
                                             , 0
                                             );
                 }
               );

   _scheduler.worker_manager().addWorker ("worker_0", 1, {}, random_ulong(), false, "host_0", fhg::util::testing::random_string());

   std::for_each ( job_ids.begin()
                 , job_ids.end()
                 , std::bind ( &sdpa::daemon::CoallocationScheduler::enqueueJob
                             , &_scheduler
                             , std::placeholders::_1
                             )
                 );

   sdpa::daemon::CoallocationScheduler::assignment_t
     assignment (_scheduler.assignJobsToWorkers());

   // at this point, all jobs are supposed to be assigned to worker_0
   BOOST_REQUIRE_EQUAL ( assignment.size(), n_jobs);

   // new worker comes up now
   _scheduler.worker_manager().addWorker ("worker_1", 1, {}, random_ulong(), false, "host_1", fhg::util::testing::random_string());

   // re-schedule all the assigned/pending jobs that are matching "worker_1"
   _scheduler.reschedule_pending_jobs_matching_worker ("worker_1");

   // re-compute costs and do a re-assignment
   assignment = _scheduler.assignJobsToWorkers();

   std::set<sdpa::worker_id_t> set_worker_0 = {worker_ids[0]};
   std::set<sdpa::worker_id_t> set_worker_1 = {worker_ids[1]};

   double sum_costs_jobs_assigned_to_worker_0 (0.0);
   double sum_costs_jobs_assigned_to_worker_1 (0.0);
   double max_job_cost ( std::max ( transfer_cost_host_0
                                  , transfer_cost_host_1
                                  )
                       + computational_cost
                       );

   for ( const std::set<sdpa::worker_id_t>& job_assigned_workers
       : assignment | boost::adaptors::map_values
       )
   {
     if (job_assigned_workers == set_worker_0)
     {
       sum_costs_jobs_assigned_to_worker_0 += transfer_cost_host_0 + computational_cost;
     }
     else
       if (job_assigned_workers == set_worker_1)
       {
         sum_costs_jobs_assigned_to_worker_1 += transfer_cost_host_1 + computational_cost;
       }
       else
       {
         throw std::runtime_error ("unexpected job assignment");
       }
   }

   BOOST_REQUIRE_LE ( std::abs ( sum_costs_jobs_assigned_to_worker_1
                               - sum_costs_jobs_assigned_to_worker_0
                               )
                    , max_job_cost
                    );
}

BOOST_FIXTURE_TEST_CASE (no_assignment_if_not_enough_memory, fixture_scheduler_and_requirements)
{
  unsigned long avail_mem (random_ulong());
  if (avail_mem > 0) avail_mem--;

  _scheduler.worker_manager().addWorker ( "worker_0"
                                        , 1
                                        , {}
                                        , avail_mem
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  const sdpa::job_id_t job_id;

  add_job (job_id, job_requirements_t ( {}
                                      , we::type::schedule_data()
                                      , null_transfer_cost
                                      , computational_cost
                                      , avail_mem + 1
                                      )
          );

  _scheduler.enqueueJob (job_id);
  sdpa::daemon::CoallocationScheduler::assignment_t
    assignment (_scheduler.assignJobsToWorkers());

  BOOST_REQUIRE (assignment.empty());
}

BOOST_FIXTURE_TEST_CASE (allocate_multiple_jobs_with_memory_requirements, fixture_scheduler_and_requirements)
{
  unsigned int size_0 (1000), size_1 (2000);
  std::set<sdpa::worker_id_t> set_0 {"worker_0"}, set_1 {"worker_1"};

  _scheduler.worker_manager().addWorker ( "worker_0"
                                        , 1
                                        , {}
                                        , size_0
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );

  _scheduler.worker_manager().addWorker ( "worker_1"
                                        , 1
                                        , {}
                                        , size_1
                                        , false
                                        , fhg::util::testing::random_string()
                                        , fhg::util::testing::random_string()
                                        );


  const sdpa::job_id_t job_id_0 ("job_0"), job_id_1 ("job_1");

  add_job (job_id_0, job_requirements_t ( {}
                                        , we::type::schedule_data()
                                        , null_transfer_cost
                                        , computational_cost
                                        , size_0
                                        )
          );

  add_job (job_id_1, job_requirements_t ( {}
                                         , we::type::schedule_data()
                                         , null_transfer_cost
                                         , computational_cost
                                         , size_1
                                         )
           );


  _scheduler.enqueueJob (job_id_0);
  _scheduler.enqueueJob (job_id_1);

  sdpa::daemon::CoallocationScheduler::assignment_t
    assignment (_scheduler.assignJobsToWorkers());

  BOOST_REQUIRE (!assignment.empty());
  BOOST_REQUIRE (assignment.count (job_id_0));
  BOOST_REQUIRE (assignment.count (job_id_1));

  BOOST_REQUIRE_EQUAL (assignment.at (job_id_0), set_0);
  BOOST_REQUIRE_EQUAL (assignment.at (job_id_1), set_1);

  _scheduler.releaseReservation (job_id_0);
  _scheduler.releaseReservation (job_id_1);

  _scheduler.enqueueJob (job_id_1);
  _scheduler.enqueueJob (job_id_0);

  assignment.clear();
  assignment = _scheduler.assignJobsToWorkers();

  BOOST_REQUIRE (!assignment.empty());
  BOOST_REQUIRE (assignment.count (job_id_0));
  BOOST_REQUIRE (assignment.count (job_id_1));

  BOOST_REQUIRE_EQUAL (assignment.at (job_id_0), set_0);
  BOOST_REQUIRE_EQUAL (assignment.at (job_id_1), set_1);
}
