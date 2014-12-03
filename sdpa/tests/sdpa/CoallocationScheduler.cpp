#define BOOST_TEST_MODULE TestScheduler
#include <utils.hpp>
#include <boost/test/unit_test.hpp>

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
#include <fhg/util/boost/test/printer/set.hpp>
#include <fhg/util/boost/test.hpp>
#include <fhg/util/random_string.hpp>

#include <iostream>
#include <functional>
#include <random>

struct serveJob_checking_scheduler_and_job_manager
{
  serveJob_checking_scheduler_and_job_manager()
    : _scheduler
      ( std::bind (&serveJob_checking_scheduler_and_job_manager::serveJob, this, std::placeholders::_1, std::placeholders::_2)
      , std::bind (&serveJob_checking_scheduler_and_job_manager::requirements, this, std::placeholders::_1)
      )
  {}

  sdpa::daemon::CoallocationScheduler _scheduler;

  ~serveJob_checking_scheduler_and_job_manager()
  {
    BOOST_REQUIRE (_expected_serveJob_calls.empty());
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

  void serveJob
    (const sdpa::worker_id_list_t& worker_list, const sdpa::job_id_t& jobId)
  {
    BOOST_REQUIRE_GE (_expected_serveJob_calls.count (jobId), 1);
    BOOST_CHECK_EQUAL (_expected_serveJob_calls[jobId].first, worker_list.size());

    BOOST_REQUIRE_EQUAL
      ( std::accumulate ( std::begin (_expected_serveJob_calls.at (jobId).second)
                        , std::end (_expected_serveJob_calls.at (jobId).second)
                        , 0
                        , [&worker_list] (int assignment_counter, const std::set<sdpa::worker_id_t>& expected_workers)
                          {return ( std::all_of ( worker_list.begin()
                                                , worker_list.end()
                                                , [&expected_workers] (const sdpa::worker_id_t& worker)
                                                  {return expected_workers.count (worker);}
                                                )
                                  + assignment_counter
                                  );
                          }
                        )
         , 1
        );

    _expected_serveJob_calls.erase (_expected_serveJob_calls.find (jobId));
  }

  typedef std::set<std::set<sdpa::worker_id_t>> set_set_worker_id_t;

  void expect_serveJob_call (sdpa::job_id_t id, std::set<sdpa::worker_id_t> list)
  {
    _expected_serveJob_calls.emplace
      (id, std::make_pair<std::size_t, set_set_worker_id_t> (list.size(), {list}));
  }

  void expect_serveJob_call (sdpa::job_id_t id, std::size_t count, set_set_worker_id_t list)
  {
    _expected_serveJob_calls.emplace (id, std::make_pair (count, list));
  }

  std::map<sdpa::job_id_t, std::pair<std::size_t, set_set_worker_id_t>>
    _expected_serveJob_calls;
};

namespace
{
  const double computational_cost = 1.0;

  job_requirements_t require (std::string name_1)
  {
    return {{we::type::requirement_t (name_1, true)}, we::type::schedule_data(), null_transfer_cost, computational_cost};
  }

  job_requirements_t require (std::string name, unsigned long workers)
  {
    return {{we::type::requirement_t (name, true)}, we::type::schedule_data (workers), null_transfer_cost, computational_cost};
  }

  job_requirements_t require (unsigned long workers)
  {
    return {{}, we::type::schedule_data (workers), null_transfer_cost, computational_cost};
  }

  job_requirements_t no_requirements()
  {
    return {{}, we::type::schedule_data(), null_transfer_cost, computational_cost};
  }
}

BOOST_FIXTURE_TEST_CASE (testLoadBalancing, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, {}, false, fhg::util::random_string());
  _scheduler.worker_manager().addWorker ("worker_1", 1, {}, false, fhg::util::random_string());

  add_job ("job_0", no_requirements());
  add_job ("job_1", no_requirements());
  add_job ("job_2", no_requirements());

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");
  _scheduler.enqueueJob ("job_2");

  expect_serveJob_call ("job_0", 1, {{"worker_0"},{"worker_1"}});
  expect_serveJob_call ("job_1", 1, {{"worker_0"},{"worker_1"}});

  _scheduler.assignJobsToWorkers();



  _scheduler.releaseReservation ("job_0");
  _scheduler.releaseReservation ("job_1");


  expect_serveJob_call ("job_2", 1, {{"worker_0"},{"worker_1"}});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE (tesLBOneWorkerJoinsLater, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, {}, false, fhg::util::random_string());

  add_job ("job_0", no_requirements());
  add_job ("job_1", no_requirements());

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");


  expect_serveJob_call ("job_0", {"worker_0"});

  _scheduler.assignJobsToWorkers();

  _scheduler.worker_manager().addWorker ("worker_1", 1, {}, false, fhg::util::random_string());
  _scheduler.reschedule_pending_jobs_matching_worker ("worker_1");

  expect_serveJob_call ("job_1", {"worker_1"});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE (tesLBOneWorkerGainsCpbLater, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, {sdpa::capability_t ("C", "worker_0")}, false, fhg::util::random_string());
  _scheduler.worker_manager().addWorker ("worker_1", 1, {}, false, fhg::util::random_string());

  add_job ("job_0", require ("C"));
  add_job ("job_1", require ("C"));

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");


  expect_serveJob_call ("job_0", {"worker_0"});

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().findWorker ("worker_1")->addCapabilities ({sdpa::capability_t ("C", "worker_1")});
  _scheduler.reschedule_pending_jobs_matching_worker ("worker_1");

  expect_serveJob_call ("job_1", {"worker_1"});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE (testCoallocSched, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("A0", 1, {sdpa::capability_t ("A", "A0")}, false, fhg::util::random_string());
  _scheduler.worker_manager().addWorker ("B0", 1, {sdpa::capability_t ("B", "B0")}, false, fhg::util::random_string());
  _scheduler.worker_manager().addWorker ("A1", 1, {sdpa::capability_t ("A", "A1")}, false, fhg::util::random_string());
  _scheduler.worker_manager().addWorker ("B1", 1, {sdpa::capability_t ("B", "B1")}, false, fhg::util::random_string());

  add_job ("2A", require ("A", 2));
  add_job ("2B", require ("B", 2));

  _scheduler.enqueueJob ("2A");
  _scheduler.enqueueJob ("2B");


  expect_serveJob_call ("2A", {"A0", "A1"});
  expect_serveJob_call ("2B", {"B0", "B1"});

  _scheduler.assignJobsToWorkers();


  add_job ("1A", require ("A", 1));

  _scheduler.enqueueJob ("1A");

  _scheduler.assignJobsToWorkers();


  _scheduler.releaseReservation ("2A");

  expect_serveJob_call ("1A", 1, {{"A0"}, {"A1"}});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE (tesLBStopRestartWorker, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, {}, false, fhg::util::random_string());
  _scheduler.worker_manager().addWorker ("worker_1", 1, {}, false, fhg::util::random_string());

  add_job ("job_0", no_requirements());
  add_job ("job_1", no_requirements());

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");

  expect_serveJob_call ("job_0", 1, {{"worker_0"}, {"worker_1"}});
  expect_serveJob_call ("job_1", 1, {{"worker_0"}, {"worker_1"}});

  _scheduler.assignJobsToWorkers();


  _scheduler.releaseReservation ("job_0");
  _scheduler.worker_manager().deleteWorker ("worker_0");
  _scheduler.enqueueJob ("job_0");

  _scheduler.worker_manager().addWorker ("worker_0", 1, {}, false, fhg::util::random_string());
  _scheduler.reschedule_pending_jobs_matching_worker ("worker_0");

  expect_serveJob_call ("job_0",  1, {{"worker_0"}, {"worker_1"}});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE
  (not_schedulable_job_does_not_block_others, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker", 1, {}, false, fhg::util::random_string());

  add_job ("2", require (2));
  _scheduler.enqueueJob ("2");

  _scheduler.assignJobsToWorkers();

  add_job ("1", require (1));
  _scheduler.enqueueJob ("1");

  expect_serveJob_call ("1", {"worker"});
  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE
  (multiple_job_submissions_no_requirements, serveJob_checking_scheduler_and_job_manager)
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  _scheduler.worker_manager().addWorker (worker_id, boost::none, {}, true, fhg::util::random_string());

  sdpa::job_id_t const job_id_0 (utils::random_peer_name());
  add_job (job_id_0, no_requirements());
  _scheduler.enqueueJob (job_id_0);
  expect_serveJob_call (job_id_0, {worker_id});
  _scheduler.assignJobsToWorkers();

  sdpa::job_id_t const job_id_1 (utils::random_peer_name());
  add_job (job_id_1, no_requirements());
  _scheduler.enqueueJob (job_id_1);
  expect_serveJob_call (job_id_1, {worker_id});
  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE ( multiple_job_submissions_with_no_children_allowed
                        , serveJob_checking_scheduler_and_job_manager
                        )
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  _scheduler.worker_manager().addWorker (worker_id, boost::none, {}, false, fhg::util::random_string());

  sdpa::job_id_t const job_id_0 (utils::random_peer_name());
  add_job (job_id_0, no_requirements());
  _scheduler.enqueueJob (job_id_0);
  expect_serveJob_call (job_id_0, {worker_id});
  _scheduler.assignJobsToWorkers();

  sdpa::job_id_t const job_id_1 (utils::random_peer_name());
  add_job (job_id_1, no_requirements());
  _scheduler.enqueueJob (job_id_1);
  _scheduler.assignJobsToWorkers();

  _scheduler.worker_manager().findWorker (worker_id)->deleteJob (job_id_0);

  expect_serveJob_call (job_id_1, {worker_id});
  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE
  (multiple_worker_job_submissions_with_requirements, serveJob_checking_scheduler_and_job_manager)
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  _scheduler.worker_manager().addWorker
    ( worker_id, boost::none, { sdpa::capability_t("A", worker_id)
                              , sdpa::capability_t("B", worker_id)
                              }
                              , true
                              , fhg::util::random_string()
    );

  sdpa::job_id_t const job_id_0 (utils::random_peer_name());
  add_job (job_id_0, require ("A"));
  _scheduler.enqueueJob (job_id_0);
  expect_serveJob_call (job_id_0, {worker_id});
  _scheduler.assignJobsToWorkers();

  sdpa::job_id_t const job_id_1 (utils::random_peer_name());
  add_job (job_id_1, require ("B"));
  _scheduler.enqueueJob (job_id_1);
  expect_serveJob_call (job_id_1, {worker_id});
  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE ( multiple_worker_job_submissions_with_requirements_no_children_allowed
                        , serveJob_checking_scheduler_and_job_manager
                        )
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  _scheduler.worker_manager().addWorker ( worker_id
                                        , boost::none
                                        , { sdpa::capability_t ("A", worker_id)
                                          , sdpa::capability_t ("B", worker_id)
                                          }
                                        , false
                                        , fhg::util::random_string()
                                        );

  sdpa::job_id_t const job_id_0 (utils::random_peer_name());
  add_job (job_id_0, require ("A"));
  _scheduler.enqueueJob (job_id_0);
  expect_serveJob_call (job_id_0, {worker_id});
  _scheduler.assignJobsToWorkers();

  sdpa::job_id_t const job_id_1 (utils::random_peer_name());
  add_job (job_id_1, require ("B"));
  _scheduler.enqueueJob (job_id_1);
  _scheduler.assignJobsToWorkers();

  _scheduler.worker_manager().findWorker (worker_id)->deleteJob (job_id_0);

  expect_serveJob_call (job_id_1, {worker_id});
  _scheduler.assignJobsToWorkers();
}

struct fixture_minimal_cost_assignment
{
  fixture_minimal_cost_assignment()
  : _scheduler
      ( [](const sdpa::worker_id_list_t&, const sdpa::job_id_t&) {}
      , [](const sdpa::job_id_t&) {return no_requirements();}
      )
  {
    _scheduler.worker_manager().addWorker ("worker_01", 1, {}, false, "node1");
    _scheduler.worker_manager().addWorker ("worker_02", 1, {}, false, "node1");
    _scheduler.worker_manager().addWorker ("worker_03", 1, {}, false, "node1");
    _scheduler.worker_manager().addWorker ("worker_04", 1, {}, false, "node1");
    _scheduler.worker_manager().addWorker ("worker_05", 1, {}, false, "node2");
    _scheduler.worker_manager().addWorker ("worker_06", 1, {}, false, "node2");
    _scheduler.worker_manager().addWorker ("worker_07", 1, {}, false, "node2");
    _scheduler.worker_manager().addWorker ("worker_08", 1, {}, false, "node2");
    _scheduler.worker_manager().addWorker ("worker_09", 1, {}, false, "node3");
    _scheduler.worker_manager().addWorker ("worker_10", 1, {}, false, "node3");
    _scheduler.worker_manager().addWorker ("worker_11", 1, {}, false, "node3");
    _scheduler.worker_manager().addWorker ("worker_12", 1, {}, false, "node3");
    _scheduler.worker_manager().addWorker ("worker_13", 1, {}, false, "node4");
    _scheduler.worker_manager().addWorker ("worker_14", 1, {}, false, "node4");
    _scheduler.worker_manager().addWorker ("worker_15", 1, {}, false, "node4");
    _scheduler.worker_manager().addWorker ("worker_16", 1, {}, false, "node4");
    _scheduler.worker_manager().addWorker ("worker_17", 1, {}, false, "node5");
    _scheduler.worker_manager().addWorker ("worker_18", 1, {}, false, "node5");
    _scheduler.worker_manager().addWorker ("worker_19", 1, {}, false, "node5");
    _scheduler.worker_manager().addWorker ("worker_20", 1, {}, false, "node5");
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
    { {20, {"worker_20", "node_5"}}
    , {19, {"worker_19", "node_5"}}
    , {18, {"worker_18", "node_5"}}
    , {17, {"worker_17", "node_5"}}
    , {16, {"worker_16", "node_4"}}
    , {15, {"worker_15", "node_4"}}
    , {14, {"worker_14", "node_4"}}
    , {13, {"worker_13", "node_4"}}
    , {12, {"worker_12", "node_3"}}
    , {11, {"worker_11", "node_3"}}
    , {10, {"worker_10", "node_3"}}
    , { 9, {"worker_09", "node_3"}}
    , { 8, {"worker_08", "node_2"}}
    , { 7, {"worker_07", "node_2"}}
    , { 6, {"worker_06", "node_2"}}
    , { 5, {"worker_05", "node_2"}}
    , { 4, {"worker_04", "node_1"}}
    , { 3, {"worker_03", "node_1"}}
    , { 2, {"worker_02", "node_1"}}
    , { 1, {"worker_01", "node_1"}}
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
    { {20, {"worker_20", "node_5"}}
    , {19, {"worker_19", "node_5"}}
    , {18, {"worker_18", "node_5"}}
    , {17, {"worker_17", "node_5"}}
    , {16, {"worker_16", "node_4"}}
    , {15, {"worker_15", "node_4"}}
    , {14, {"worker_14", "node_4"}}
    , {13, {"worker_13", "node_4"}}
    , {12, {"worker_12", "node_3"}}
    , {11, {"worker_11", "node_3"}}
    , {10, {"worker_10", "node_3"}}
    , { 9, {"worker_09", "node_3"}}
    , { 8, {"worker_08", "node_2"}}
    , { 7, {"worker_07", "node_2"}}
    , { 6, {"worker_06", "node_2"}}
    , { 5, {"worker_05", "node_2"}}
    , { 4, {"worker_04", "node_1"}}
    , { 3, {"worker_03", "node_1"}}
    , { 2, {"worker_02", "node_1"}}
    , { 1, {"worker_01", "node_1"}}
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
    { {1, {"worker_20", "node_5"}}
    , {1, {"worker_19", "node_5"}}
    , {1, {"worker_18", "node_5"}}
    , {1, {"worker_17", "node_5"}}
    , {1, {"worker_16", "node_4"}}
    , {1, {"worker_15", "node_4"}}
    , {1, {"worker_14", "node_4"}}
    , {1, {"worker_13", "node_4"}}
    , {1, {"worker_12", "node_3"}}
    , {1, {"worker_11", "node_3"}}
    , {1, {"worker_10", "node_3"}}
    , {1, {"worker_09", "node_3"}}
    , {1, {"worker_08", "node_2"}}
    , {1, {"worker_07", "node_2"}}
    , {1, {"worker_06", "node_2"}}
    , {1, {"worker_05", "node_2"}}
    , {1, {"worker_04", "node_1"}}
    , {1, {"worker_03", "node_1"}}
    , {1, {"worker_02", "node_1"}}
    , {1, {"worker_01", "node_1"}}
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
    { {1, {"worker_20", "node_5"}}
    , {1, {"worker_19", "node_5"}}
    , {1, {"worker_18", "node_5"}}
    , {1, {"worker_17", "node_5"}}
    , {1, {"worker_16", "node_4"}}
    , {1, {"worker_15", "node_4"}}
    , {1, {"worker_14", "node_4"}}
    , {1, {"worker_13", "node_4"}}
    , {1, {"worker_12", "node_3"}}
    , {1, {"worker_11", "node_3"}}
    , {1, {"worker_10", "node_3"}}
    , {1, {"worker_09", "node_3"}}
    , {1, {"worker_08", "node_2"}}
    , {1, {"worker_07", "node_2"}}
    , {1, {"worker_06", "node_2"}}
    , {1, {"worker_05", "node_2"}}
    , {1, {"worker_04", "node_1"}}
    , {1, {"worker_03", "node_1"}}
    , {1, {"worker_02", "node_1"}}
    , {1, {"worker_01", "node_1"}}
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
    _scheduler ( std::bind ( &serve_job_and_check_for_minimal_cost_assignement::serve_and_check_assignment
                           , this
                           , test_transfer_cost
                           , worker_ids
                           , std::placeholders::_1
                           , std::placeholders::_2
                           )
                , [n_req_workers, &test_transfer_cost] (const sdpa::job_id_t&)
                  {
                    return job_requirements_t ( {}
                                              , we::type::schedule_data (n_req_workers)
                                              , test_transfer_cost
                                              , computational_cost
                                              );
                  }
                );

  for (const sdpa::worker_id_t worker_id : worker_ids)
  {
    _scheduler.worker_manager().addWorker (worker_id, 1, {}, false, worker_id);
  }

  const sdpa::job_id_t job_id (fhg::util::random_string());
  _scheduler.enqueueJob (job_id);

  _scheduler.assignJobsToWorkers();

  // require the job to be scheduled
  BOOST_REQUIRE_EQUAL (_scheduler.delete_job (job_id), 0);
}

BOOST_FIXTURE_TEST_CASE ( no_coallocation_job_with_requirements_is_assigned_if_not_all_workers_are_leaves
                        , serveJob_checking_scheduler_and_job_manager
                        )
{
  sdpa::worker_id_t const agent_id (utils::random_peer_name());

  _scheduler.worker_manager().addWorker ( agent_id
                                        , boost::none
                                        , {sdpa::capability_t ("A", utils::random_peer_name())}
                                        , true
                                        , fhg::util::random_string()
                                        );

  sdpa::worker_id_t const worker_id (utils::random_peer_name());
  _scheduler.worker_manager().addWorker ( worker_id
                                        , boost::none
                                        , {sdpa::capability_t ("A", worker_id)}
                                        , false
                                        , fhg::util::random_string()
                                        );

  sdpa::job_id_t const job_id_0 (utils::random_peer_name());
  add_job (job_id_0, require ("A", 2));
  _scheduler.enqueueJob (job_id_0);

  // no serveJob expected
  _scheduler.assignJobsToWorkers();

  BOOST_REQUIRE (_scheduler.delete_job (job_id_0));
}

BOOST_FIXTURE_TEST_CASE ( no_coallocation_job_without_requirements_is_assigned_if_not_all_workers_are_leaves
                        , serveJob_checking_scheduler_and_job_manager
                        )
{
  sdpa::worker_id_t const agent_id (utils::random_peer_name());
  _scheduler.worker_manager().addWorker ( agent_id
                                        , boost::none
                                        , {}
                                        , true
                                        , fhg::util::random_string()
                                        );

  sdpa::worker_id_t const worker_id (utils::random_peer_name());
  _scheduler.worker_manager().addWorker ( worker_id
                                        , boost::none
                                        , {}
                                        , false
                                        , fhg::util::random_string()
                                        );

  sdpa::job_id_t const job_id_0 (utils::random_peer_name());

  add_job (job_id_0, require (2));
  _scheduler.enqueueJob (job_id_0);

  // no serveJob expected
  _scheduler.assignJobsToWorkers();

  BOOST_REQUIRE (_scheduler.delete_job (job_id_0));
}
