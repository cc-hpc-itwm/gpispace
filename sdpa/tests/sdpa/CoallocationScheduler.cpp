#define BOOST_TEST_MODULE TestScheduler
#include <boost/test/unit_test.hpp>

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>

struct serveJob_checking_scheduler_and_job_manager
{
  serveJob_checking_scheduler_and_job_manager()
    : _scheduler
      ( boost::bind (&serveJob_checking_scheduler_and_job_manager::serveJob, this, _1, _2)
      , boost::bind (&serveJob_checking_scheduler_and_job_manager::requirements, this, _1)
      )
  {}

  sdpa::daemon::CoallocationScheduler _scheduler;

  ~serveJob_checking_scheduler_and_job_manager()
  {
    BOOST_REQUIRE (_expected_serveJob_calls.empty());
  }

  void add_job (const sdpa::job_id_t& job_id, const job_requirements_t& reqs)
  {
    if (!_requirements.insert (std::make_pair (job_id, reqs)).second)
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
    _expected_serveJob_calls.insert
      (std::make_pair (id, std::make_pair (count, list)));
  }
  void expect_serveJob_call (sdpa::job_id_t id, std::set<sdpa::worker_id_t> list)
  {
    expect_serveJob_call (id, list.size(), list);
  }

  std::map<sdpa::job_id_t, std::pair<std::size_t, std::set<sdpa::worker_id_t> > >
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
  _scheduler.worker_manager().addWorker ("worker_0", 1, {sdpa::capability_t ("C", "worker_0")});
  _scheduler.worker_manager().addWorker ("worker_1", 1, {sdpa::capability_t ("C", "worker_1")});

  add_job ("job_0", require ("C"));
  add_job ("job_1", require ("C"));
  add_job ("job_2", require ("C"));

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
  _scheduler.worker_manager().addWorker ("worker_0", 1, {sdpa::capability_t ("C", "worker_0")});

  add_job ("job_0", require ("C"));
  add_job ("job_1", require ("C"));

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");


  expect_serveJob_call ("job_0", {"worker_0"});

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().addWorker ("worker_1", 1, {sdpa::capability_t ("C", "worker_1")});

  expect_serveJob_call ("job_1", {"worker_1"});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE (tesLBOneWorkerGainsCpbLater, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, {sdpa::capability_t ("C", "worker_0")});
  _scheduler.worker_manager().addWorker ("worker_1", 1);

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
  _scheduler.worker_manager().addWorker ("0", 1, {sdpa::capability_t ("A", "0")});
  _scheduler.worker_manager().addWorker ("1", 1, {sdpa::capability_t ("B", "1")});
  _scheduler.worker_manager().addWorker ("3", 1, {sdpa::capability_t ("A", "3")});
  _scheduler.worker_manager().addWorker ("4", 1, {sdpa::capability_t ("B", "4")});

  add_job ("job_0", require ("A", 2));
  add_job ("job_1", require ("B", 2));

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");


  expect_serveJob_call ("job_0", {"0", "3"});
  expect_serveJob_call ("job_1", {"1", "10"});

  _scheduler.assignJobsToWorkers();


  add_job ("job_3", require ("A", 1));

  _scheduler.enqueueJob ("job_3");

  _scheduler.assignJobsToWorkers();

  _scheduler.worker_manager().findWorker ("0")->deleteJob ("job_0");
  _scheduler.worker_manager().findWorker ("3")->deleteJob ("job_0");

  _scheduler.releaseReservation ("job_0");

  expect_serveJob_call ("job_3", 1, {"0", "3"});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE (tesLBStopRestartWorker, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, {sdpa::capability_t ("C", "worker_0")});
  _scheduler.worker_manager().addWorker ("worker_1", 1, {sdpa::capability_t ("C", "worker_1")});
  _scheduler.worker_manager().addWorker ("worker_2", 1, {sdpa::capability_t ("C", "worker_2")});
  _scheduler.worker_manager().addWorker ("worker_3", 1, {sdpa::capability_t ("C", "worker_3")});
  _scheduler.worker_manager().addWorker ("worker_4", 1, {sdpa::capability_t ("C", "worker_4")});
  _scheduler.worker_manager().addWorker ("worker_5", 1, {sdpa::capability_t ("C", "worker_5")});
  _scheduler.worker_manager().addWorker ("worker_6", 1, {sdpa::capability_t ("C", "worker_6")});
  _scheduler.worker_manager().addWorker ("worker_7", 1, {sdpa::capability_t ("C", "worker_7")});
  _scheduler.worker_manager().addWorker ("worker_8", 1, {sdpa::capability_t ("C", "worker_8")});
  _scheduler.worker_manager().addWorker ("worker_9", 1, {sdpa::capability_t ("C", "worker_9")});

  add_job ("job_0", require ("C"));
  add_job ("job_1", require ("C"));
  add_job ("job_2", require ("C"));
  add_job ("job_3", require ("C"));
  add_job ("job_4", require ("C"));
  add_job ("job_5", require ("C"));
  add_job ("job_6", require ("C"));
  add_job ("job_7", require ("C"));
  add_job ("job_8", require ("C"));
  add_job ("job_9", require ("C"));

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");
  _scheduler.enqueueJob ("job_2");
  _scheduler.enqueueJob ("job_3");
  _scheduler.enqueueJob ("job_4");
  _scheduler.enqueueJob ("job_5");
  _scheduler.enqueueJob ("job_6");
  _scheduler.enqueueJob ("job_7");
  _scheduler.enqueueJob ("job_8");
  _scheduler.enqueueJob ("job_9");


  expect_serveJob_call ("job_0", {"worker_9"});
  expect_serveJob_call ("job_1", {"worker_8"});
  expect_serveJob_call ("job_2", {"worker_7"});
  expect_serveJob_call ("job_3", {"worker_5"});
  expect_serveJob_call ("job_4", {"worker_4"});
  expect_serveJob_call ("job_5", {"worker_6"});
  expect_serveJob_call ("job_6", {"worker_3"});
  expect_serveJob_call ("job_7", {"worker_2"});
  expect_serveJob_call ("job_8", {"worker_1"});
  expect_serveJob_call ("job_9", {"worker_0"});

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().findWorker ("worker_9")->deleteJob ("job_0");
  _scheduler.releaseReservation ("job_0");
  _scheduler.worker_manager().deleteWorker ("worker_9");
  _scheduler.enqueueJob ("job_0");

  _scheduler.worker_manager().addWorker("worker_9", 1, {sdpa::capability_t ("C", "worker_9")});

  expect_serveJob_call ("job_0", {"worker_9"});

  _scheduler.assignJobsToWorkers();
}

BOOST_FIXTURE_TEST_CASE
  (not_schedulable_job_does_not_block_others, serveJob_checking_scheduler_and_job_manager)
{
  _scheduler.worker_manager().addWorker ("worker", 1);

  add_job ("2", require (2));
  _scheduler.enqueueJob ("2");

  _scheduler.assignJobsToWorkers();

  add_job ("1", require (1));
  _scheduler.enqueueJob ("1");

  expect_serveJob_call ("1", {"worker"});
  _scheduler.assignJobsToWorkers();
}
