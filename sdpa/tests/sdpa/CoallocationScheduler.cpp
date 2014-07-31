#define BOOST_TEST_MODULE TestScheduler
#include <boost/test/unit_test.hpp>

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
#include <fhg/util/random_string.hpp>
#include <functional>

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
    for (sdpa::worker_id_t worker : worker_list)
    {
      BOOST_REQUIRE (_expected_serveJob_calls[jobId].second.count (worker));
    }

    _expected_serveJob_calls.erase (_expected_serveJob_calls.find (jobId));
  }

  void expect_serveJob_call
    (sdpa::job_id_t id, std::size_t count, std::set<sdpa::worker_id_t> list)
  {
    _expected_serveJob_calls.emplace (id, std::make_pair (count, list));
  }
  void expect_serveJob_call (sdpa::job_id_t id, std::set<sdpa::worker_id_t> list)
  {
    expect_serveJob_call (id, list.size(), list);
  }

  std::map<sdpa::job_id_t, std::pair<std::size_t, std::set<sdpa::worker_id_t>>>
    _expected_serveJob_calls;
};

namespace
{
  job_requirements_t require (std::string name_1)
  {
    return {{we::type::requirement_t (name_1, true)}, we::type::schedule_data()};
  }

  job_requirements_t require (std::string name, unsigned long workers)
  {
    return {{we::type::requirement_t (name, true)}, we::type::schedule_data (workers)};
  }

  job_requirements_t require (unsigned long workers)
  {
    return {{}, we::type::schedule_data (workers)};
  }
}

BOOST_FIXTURE_TEST_CASE (testLoadBalancing, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, {}, fhg::util::random_string());
  _scheduler.worker_manager().addWorker ("worker_1", 1, {}, fhg::util::random_string());

  add_job ("job_0", {});
  add_job ("job_1", {});
  add_job ("job_2", {});

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");
  _scheduler.enqueueJob ("job_2");

  expect_serveJob_call ("job_0", {"worker_0"});
  expect_serveJob_call ("job_1", {"worker_1"});

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().findWorker ("worker_0")->deleteJob ("job_0");
  _scheduler.worker_manager().findWorker ("worker_1")->deleteJob ("job_1");

  _scheduler.releaseReservation ("job_0");
  _scheduler.releaseReservation ("job_1");


  expect_serveJob_call ("job_2", {"worker_0"});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE (tesLBOneWorkerJoinsLater, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, {}, fhg::util::random_string());

  add_job ("job_0", {});
  add_job ("job_1", {});

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");


  expect_serveJob_call ("job_0", {"worker_0"});

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().addWorker ("worker_1", 1, {}, fhg::util::random_string());

  expect_serveJob_call ("job_1", {"worker_1"});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE (tesLBOneWorkerGainsCpbLater, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, {sdpa::capability_t ("C", "worker_0")}, fhg::util::random_string());
  _scheduler.worker_manager().addWorker ("worker_1", 1, {}, fhg::util::random_string());

  add_job ("job_0", require ("C"));
  add_job ("job_1", require ("C"));

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");


  expect_serveJob_call ("job_0", {"worker_0"});

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().findWorker ("worker_1")->addCapabilities ({sdpa::capability_t ("C", "worker_1")});

  expect_serveJob_call ("job_1", {"worker_1"});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE (testCoallocSched, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("A0", 1, {sdpa::capability_t ("A", "A0")}, fhg::util::random_string());
  _scheduler.worker_manager().addWorker ("B0", 1, {sdpa::capability_t ("B", "B0")}, fhg::util::random_string());
  _scheduler.worker_manager().addWorker ("A1", 1, {sdpa::capability_t ("A", "A1")}, fhg::util::random_string());
  _scheduler.worker_manager().addWorker ("B1", 1, {sdpa::capability_t ("B", "B1")}, fhg::util::random_string());

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

  _scheduler.worker_manager().findWorker ("A0")->deleteJob ("2A");
  _scheduler.worker_manager().findWorker ("A1")->deleteJob ("2A");

  _scheduler.releaseReservation ("2A");

  expect_serveJob_call ("1A", 1, {"A0", "A1"});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE (tesLBStopRestartWorker, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, {}, fhg::util::random_string());
  _scheduler.worker_manager().addWorker ("worker_1", 1, {}, fhg::util::random_string());

  add_job ("job_0", {});
  add_job ("job_1", {});

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");

  expect_serveJob_call ("job_0", {"worker_0"});
  expect_serveJob_call ("job_1", {"worker_1"});

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().findWorker ("worker_0")->deleteJob ("job_0");
  _scheduler.releaseReservation ("job_0");
  _scheduler.worker_manager().deleteWorker ("worker_0");
  _scheduler.enqueueJob ("job_0");

  _scheduler.worker_manager().addWorker ("worker_0", 1, {}, fhg::util::random_string());

  expect_serveJob_call ("job_0", {"worker_0"});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE
  (not_schedulable_job_does_not_block_others, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker", 1, {}, fhg::util::random_string());

  add_job ("2", require (2));
  _scheduler.enqueueJob ("2");

  _scheduler.assignJobsToWorkers();

  add_job ("1", require (1));
  _scheduler.enqueueJob ("1");

  expect_serveJob_call ("1", {"worker"});
  _scheduler.assignJobsToWorkers();
}

BOOST_AUTO_TEST_CASE (scheduling_with_data_locality_different_matching_degs_different_costs)
{
  // assume we have 5 nodes and the transfer cost for each is its rank
  const std::map<std::string, double> map_host_transfer_cost
    = { {"node_1", 1.0}
      , {"node_2", 2.0}
      , {"node_3", 3.0}
      , {"node_4", 4.0}
      , {"node_5", 5.0}
      };

  // find an allocation minimizing the transfer costs for 5 workers
  const size_t n_req_workers (5);

  // assume that we have 20 workers, i.e. 4 workers per host
  // first 4 on the "node_0", the next 4 on the "node_1" and so on
  const sdpa::mmap_match_deg_worker_id_t mmap_match_deg_worker
    = { {20, {"worker_20", "node_5"}}
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

  const std::set<sdpa::worker_id_t> set_expected_assignment = { "worker_01"
                                                              , "worker_02"
                                                              , "worker_03"
                                                              , "worker_04"
                                                              , "worker_08"
                                                              };

  const std::set<sdpa::worker_id_t> set_assigned_jobs
    (sdpa::daemon::CoallocationScheduler::find_job_assignment_minimizing_memory_transfer_cost
       (mmap_match_deg_worker, n_req_workers, map_host_transfer_cost)
    );


  BOOST_REQUIRE (set_assigned_jobs == set_expected_assignment);
}

BOOST_AUTO_TEST_CASE (scheduling_with_data_locality_different_matching_degs_equal_costs)
{
  // assume we have 5 nodes and the transfer cost is the same for all hosts
  const std::map<std::string, double> map_host_transfer_cost
    = { {"node_1", 1.0}
      , {"node_2", 1.0}
      , {"node_3", 1.0}
      , {"node_4", 1.0}
      , {"node_5", 1.0}
      };

  // find an allocation minimizing the transfer costs for 5 workers
  const size_t n_req_workers (5);

  // assume that we have 20 workers, i.e. 4 workers per host
  // first 4 on "node_0", the next 4 on the "node_1" and so on
  const sdpa::mmap_match_deg_worker_id_t mmap_match_deg_worker
    = { {20, {"worker_20", "node_5"}}
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

  const std::set<sdpa::worker_id_t> set_expected_assignment = { "worker_20"
                                                              , "worker_19"
                                                              , "worker_18"
                                                              , "worker_17"
                                                              , "worker_16"
                                                              };

  const std::set<sdpa::worker_id_t> set_assigned_jobs
    (sdpa::daemon::CoallocationScheduler::find_job_assignment_minimizing_memory_transfer_cost
      (mmap_match_deg_worker, n_req_workers, map_host_transfer_cost)
    );


  BOOST_REQUIRE (set_assigned_jobs == set_expected_assignment);
}

BOOST_AUTO_TEST_CASE (scheduling_with_data_locality_equal_matching_degs_different_costs)
{
  // assume we have 5 nodes and the transfer cost is different for any host
  const std::map<std::string, double> map_host_transfer_cost
    = { {"node_1", 5.0}
      , {"node_2", 4.0}
      , {"node_3", 3.0}
      , {"node_4", 2.0}
      , {"node_5", 1.0}
      };

  // find an allocation minimizing the transfer costs for 5 workers
  const size_t n_req_workers (5);

  // assume that we have 20 workers, i.e. 4 workers per host
  // first 4 on "node_0", the next 4 on the "node_1" and so on
  const sdpa::mmap_match_deg_worker_id_t mmap_match_deg_worker
    = { {1, {"worker_20", "node_5"}}
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

  const std::set<sdpa::worker_id_t> set_expected_assignment = { "worker_20"
                                                              , "worker_19"
                                                              , "worker_18"
                                                              , "worker_17"
                                                              , "worker_13"
                                                              };

 const std::set<sdpa::worker_id_t> set_assigned_jobs
    (sdpa::daemon::CoallocationScheduler::find_job_assignment_minimizing_memory_transfer_cost
      (mmap_match_deg_worker, n_req_workers, map_host_transfer_cost)
    );

  BOOST_REQUIRE (set_assigned_jobs == set_expected_assignment);
}

BOOST_AUTO_TEST_CASE (scheduling_with_data_locality_equal_matching_degs_equal_costs)
{
  // assume we have 5 nodes and the transfer cost is the same for all hosts
  const std::map<std::string, double> map_host_transfer_cost
    = { {"node_1", 1.0}
      , {"node_2", 1.0}
      , {"node_3", 1.0}
      , {"node_4", 1.0}
      , {"node_5", 1.0}
      };

  // find an allocation minimizing the transfer costs for 5 workers
  const size_t n_req_workers (5);

  // assume that we have 20 workers, i.e. 4 workers per host
  // first 4 on "node_0", the next 4 on the "node_1" and so on
  const sdpa::mmap_match_deg_worker_id_t mmap_match_deg_worker
    = { {1, {"worker_20", "node_5"}}
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

  const std::set<sdpa::worker_id_t> set_expected_assignment = { "worker_01"
                                                              , "worker_02"
                                                              , "worker_03"
                                                              , "worker_04"
                                                              , "worker_05"
                                                              };

  const std::set<sdpa::worker_id_t> set_assigned_jobs
    (sdpa::daemon::CoallocationScheduler::find_job_assignment_minimizing_memory_transfer_cost
      (mmap_match_deg_worker, n_req_workers, map_host_transfer_cost)
    );

  BOOST_REQUIRE (set_assigned_jobs == set_expected_assignment);
}
