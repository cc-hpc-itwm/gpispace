#define BOOST_TEST_MODULE TestScheduler
#include <utils.hpp>
#include <boost/test/unit_test.hpp>

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>

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
  _scheduler.worker_manager().addWorker ("worker_0", 1, {}, false);
  _scheduler.worker_manager().addWorker ("worker_1", 1, {}, false);

  add_job ("job_0", {});
  add_job ("job_1", {});
  add_job ("job_2", {});

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");
  _scheduler.enqueueJob ("job_2");

  expect_serveJob_call ("job_0", {"worker_1"});
  expect_serveJob_call ("job_1", {"worker_0"});

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().findWorker ("worker_1")->deleteJob ("job_0");
  _scheduler.worker_manager().findWorker ("worker_0")->deleteJob ("job_1");

  _scheduler.releaseReservation ("job_0");
  _scheduler.releaseReservation ("job_1");


  expect_serveJob_call ("job_2", {"worker_1"});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE (tesLBOneWorkerJoinsLater, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, {}, false);

  add_job ("job_0", {});
  add_job ("job_1", {});

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");


  expect_serveJob_call ("job_0", {"worker_0"});

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().addWorker ("worker_1", 1, {}, false);

  expect_serveJob_call ("job_1", {"worker_1"});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE (tesLBOneWorkerGainsCpbLater, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, {sdpa::capability_t ("C", "worker_0")}, false);
  _scheduler.worker_manager().addWorker ("worker_1", 1, {}, false);

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
  _scheduler.worker_manager().addWorker ("A0", 1, {sdpa::capability_t ("A", "A0")}, false);
  _scheduler.worker_manager().addWorker ("B0", 1, {sdpa::capability_t ("B", "B0")}, false);
  _scheduler.worker_manager().addWorker ("A1", 1, {sdpa::capability_t ("A", "A1")}, false);
  _scheduler.worker_manager().addWorker ("B1", 1, {sdpa::capability_t ("B", "B1")}, false);

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
  _scheduler.worker_manager().addWorker ("worker_0", 1, {}, false);
  _scheduler.worker_manager().addWorker ("worker_1", 1, {}, false);

  add_job ("job_0", {});
  add_job ("job_1", {});

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");


  expect_serveJob_call ("job_0", {"worker_1"});
  expect_serveJob_call ("job_1", {"worker_0"});

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().findWorker ("worker_1")->deleteJob ("job_0");
  _scheduler.releaseReservation ("job_0");
  _scheduler.worker_manager().deleteWorker ("worker_1");
  _scheduler.enqueueJob ("job_0");

  _scheduler.worker_manager().addWorker ("worker_1", 1, {}, false);

  expect_serveJob_call ("job_0", {"worker_1"});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE
  (not_schedulable_job_does_not_block_others, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker", 1, {}, false);

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

  _scheduler.worker_manager().addWorker (worker_id, boost::none, {}, true);

  sdpa::job_id_t const job_id_0 (utils::random_peer_name());
  add_job (job_id_0, {});
  _scheduler.enqueueJob (job_id_0);
  expect_serveJob_call (job_id_0, {worker_id});
  _scheduler.assignJobsToWorkers();

  sdpa::job_id_t const job_id_1 (utils::random_peer_name());
  add_job (job_id_1, {});
  _scheduler.enqueueJob (job_id_1);
  expect_serveJob_call (job_id_1, {worker_id});
  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE ( multiple_job_submissions_with_no_children_allowed
                        , serveJob_checking_scheduler_and_job_manager
                        )
{
  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  _scheduler.worker_manager().addWorker (worker_id, boost::none, {}, false);

  sdpa::job_id_t const job_id_0 (utils::random_peer_name());
  add_job (job_id_0, {});
  _scheduler.enqueueJob (job_id_0);
  expect_serveJob_call (job_id_0, {worker_id});
  _scheduler.assignJobsToWorkers();

  sdpa::job_id_t const job_id_1 (utils::random_peer_name());
  add_job (job_id_1, {});
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

BOOST_FIXTURE_TEST_CASE ( no_coallocation_job_with_requirements_is_assigned_if_not_all_workers_are_leaves
                        , serveJob_checking_scheduler_and_job_manager
                        )
{
  sdpa::worker_id_t const agent_id (utils::random_peer_name());

  _scheduler.worker_manager().addWorker ( agent_id
                                        , boost::none
                                        , {sdpa::capability_t ("A", utils::random_peer_name())}
                                        , true
                                        );

  sdpa::worker_id_t const worker_id (utils::random_peer_name());
  _scheduler.worker_manager().addWorker ( worker_id
                                        , boost::none
                                        , {sdpa::capability_t ("A", worker_id)}
                                        , false
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
                                        );

  sdpa::worker_id_t const worker_id (utils::random_peer_name());
  _scheduler.worker_manager().addWorker ( worker_id
                                        , boost::none
                                        , {}
                                        , false
                                        );

  sdpa::job_id_t const job_id_0 (utils::random_peer_name());

  add_job (job_id_0, require (2));
  _scheduler.enqueueJob (job_id_0);

  // no serveJob expected
  _scheduler.assignJobsToWorkers();

  BOOST_REQUIRE (_scheduler.delete_job (job_id_0));
}
