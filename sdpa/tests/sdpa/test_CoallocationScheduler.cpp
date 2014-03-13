#define BOOST_TEST_MODULE TestScheduler
#include <boost/test/unit_test.hpp>
#include <sdpa/daemon/agent/Agent.hpp>
#include "kvs_setup_fixture.hpp"

#include <fhg/util/boost/test/printer/list.hpp>
#include <fhg/util/boost/test/printer/set.hpp>

BOOST_TEST_DONT_PRINT_LOG_VALUE (sdpa::Capability)

const std::string WORKER_CPBS[] = {"A", "B", "C"};

typedef std::map<sdpa::job_id_t, sdpa::worker_id_t> mapJob2Worker_t;

class TestAgent : public sdpa::daemon::Agent
{
public:
  typedef boost::shared_ptr<TestAgent > ptr_t;
  TestAgent( const std::string& name
             , const std::string& url
           , std::string kvs_host, std::string kvs_port
             , const sdpa::master_info_list_t& arrMasterNames
             , const boost::optional<std::string>& appGuiUrl = boost::none)
    : sdpa::daemon::Agent(name, url, kvs_host, kvs_port, arrMasterNames, appGuiUrl)
  {
  }


  void sendEventToOther(const sdpa::events::SDPAEvent::Ptr&)
  {
    throw std::runtime_error ("trying to send message in test case which should not send messages");
  }
};

struct allocate_test_agent_and_scheduler
{
  allocate_test_agent_and_scheduler()
    : _agent ("agent", "127.0.0.1", kvs_host(), kvs_port(), sdpa::master_info_list_t())
    , _scheduler
      ( boost::bind (&allocate_test_agent_and_scheduler::serveJob, this, _1, _2)
      , boost::bind (&TestAgent::findJob, &_agent, _1)
      )
  {}

  TestAgent _agent;
  sdpa::daemon::CoallocationScheduler _scheduler;

  ~allocate_test_agent_and_scheduler()
  {
    BOOST_REQUIRE (_expected_serveJob_calls.empty());
  }

  void serveJob
    (const sdpa::worker_id_list_t& worker_list, const sdpa::job_id_t& jobId)
  {
    BOOST_REQUIRE_GE (_expected_serveJob_calls.count (jobId), 1);
    BOOST_CHECK_EQUAL (_expected_serveJob_calls[jobId].first, worker_list.size());
    BOOST_FOREACH (sdpa::worker_id_t worker, worker_list)
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
  std::set<sdpa::worker_id_t> worker_list (sdpa::worker_id_t w1)
  {
    std::set<sdpa::worker_id_t> workers;
    workers.insert (w1);
    return workers;
  }
  std::set<sdpa::worker_id_t> worker_list ( sdpa::worker_id_t w1
                                          , sdpa::worker_id_t w2
                                          , sdpa::worker_id_t w3
                                          , sdpa::worker_id_t w4
                                          )
  {
    std::set<sdpa::worker_id_t> workers;
    workers.insert (w1);
    workers.insert (w2);
    workers.insert (w3);
    workers.insert (w4);
    return workers;
  }

  sdpa::capabilities_set_t capabilities ( sdpa::worker_id_t worker
                                        , std::string name_1
                                        )
  {
    sdpa::capabilities_set_t set;
    set.insert (sdpa::capability_t (name_1, worker));
    return set;
  }

  job_requirements_t require (std::string name_1)
  {
    requirement_list_t reqs;
    reqs.push_back (we::type::requirement_t (name_1, true));
    return job_requirements_t (reqs, we::type::schedule_data());
  }

  job_requirements_t require (std::string name_1, unsigned long workers)
  {
    requirement_list_t reqs;
    reqs.push_back (we::type::requirement_t (name_1, true));
    return job_requirements_t (reqs, we::type::schedule_data (workers));
  }

  job_requirements_t require (unsigned long workers)
  {
    requirement_list_t reqs;
    return job_requirements_t (reqs, we::type::schedule_data (workers));
  }
}

BOOST_FIXTURE_TEST_SUITE( test_Scheduler, allocate_test_agent_and_scheduler)

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE(testLoadBalancing)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, capabilities ("worker_0", "C"));
  _scheduler.worker_manager().addWorker ("worker_1", 1, capabilities ("worker_1", "C"));
  _scheduler.worker_manager().addWorker ("worker_2", 1, capabilities ("worker_2", "C"));
  _scheduler.worker_manager().addWorker ("worker_3", 1, capabilities ("worker_3", "C"));
  _scheduler.worker_manager().addWorker ("worker_4", 1, capabilities ("worker_4", "C"));
  _scheduler.worker_manager().addWorker ("worker_5", 1, capabilities ("worker_5", "C"));
  _scheduler.worker_manager().addWorker ("worker_6", 1, capabilities ("worker_6", "C"));
  _scheduler.worker_manager().addWorker ("worker_7", 1, capabilities ("worker_7", "C"));
  _scheduler.worker_manager().addWorker ("worker_8", 1, capabilities ("worker_8", "C"));
  _scheduler.worker_manager().addWorker ("worker_9", 1, capabilities ("worker_9", "C"));

  _agent.TEST_add_dummy_job ("job_0", require ("C"));
  _agent.TEST_add_dummy_job ("job_1", require ("C"));
  _agent.TEST_add_dummy_job ("job_2", require ("C"));
  _agent.TEST_add_dummy_job ("job_3", require ("C"));
  _agent.TEST_add_dummy_job ("job_4", require ("C"));
  _agent.TEST_add_dummy_job ("job_5", require ("C"));
  _agent.TEST_add_dummy_job ("job_6", require ("C"));
  _agent.TEST_add_dummy_job ("job_7", require ("C"));
  _agent.TEST_add_dummy_job ("job_8", require ("C"));
  _agent.TEST_add_dummy_job ("job_9", require ("C"));
  _agent.TEST_add_dummy_job ("job_10", require ("C"));
  _agent.TEST_add_dummy_job ("job_11", require ("C"));
  _agent.TEST_add_dummy_job ("job_12", require ("C"));
  _agent.TEST_add_dummy_job ("job_13", require ("C"));
  _agent.TEST_add_dummy_job ("job_14", require ("C"));

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
  _scheduler.enqueueJob ("job_10");
  _scheduler.enqueueJob ("job_11");
  _scheduler.enqueueJob ("job_12");
  _scheduler.enqueueJob ("job_13");
  _scheduler.enqueueJob ("job_14");


  expect_serveJob_call ("job_0", worker_list ("worker_9"));
  expect_serveJob_call ("job_1", worker_list ("worker_8"));
  expect_serveJob_call ("job_2", worker_list ("worker_7"));
  expect_serveJob_call ("job_3", worker_list ("worker_5"));
  expect_serveJob_call ("job_4", worker_list ("worker_4"));
  expect_serveJob_call ("job_5", worker_list ("worker_6"));
  expect_serveJob_call ("job_6", worker_list ("worker_3"));
  expect_serveJob_call ("job_7", worker_list ("worker_2"));
  expect_serveJob_call ("job_8", worker_list ("worker_1"));
  expect_serveJob_call ("job_9", worker_list ("worker_0"));

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().findWorker ("worker_9")->deleteJob ("job_0");
  _scheduler.worker_manager().findWorker ("worker_8")->deleteJob ("job_1");
  _scheduler.worker_manager().findWorker ("worker_7")->deleteJob ("job_2");
  _scheduler.worker_manager().findWorker ("worker_5")->deleteJob ("job_3");
  _scheduler.worker_manager().findWorker ("worker_4")->deleteJob ("job_4");
  _scheduler.worker_manager().findWorker ("worker_6")->deleteJob ("job_5");
  _scheduler.worker_manager().findWorker ("worker_3")->deleteJob ("job_6");
  _scheduler.worker_manager().findWorker ("worker_2")->deleteJob ("job_7");
  _scheduler.worker_manager().findWorker ("worker_1")->deleteJob ("job_8");
  _scheduler.worker_manager().findWorker ("worker_0")->deleteJob ("job_9");

  _scheduler.releaseReservation ("job_0");
  _scheduler.releaseReservation ("job_1");
  _scheduler.releaseReservation ("job_2");
  _scheduler.releaseReservation ("job_3");
  _scheduler.releaseReservation ("job_4");
  _scheduler.releaseReservation ("job_5");
  _scheduler.releaseReservation ("job_6");
  _scheduler.releaseReservation ("job_7");
  _scheduler.releaseReservation ("job_8");
  _scheduler.releaseReservation ("job_9");


  expect_serveJob_call ("job_10", worker_list ("worker_9"));
  expect_serveJob_call ("job_11", worker_list ("worker_8"));
  expect_serveJob_call ("job_12", worker_list ("worker_7"));
  expect_serveJob_call ("job_13", worker_list ("worker_5"));
  expect_serveJob_call ("job_14", worker_list ("worker_4"));

  _scheduler.assignJobsToWorkers();
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerJoinsLater)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, capabilities ("worker_0", "C"));
  _scheduler.worker_manager().addWorker ("worker_1", 1, capabilities ("worker_1", "C"));
  _scheduler.worker_manager().addWorker ("worker_2", 1, capabilities ("worker_2", "C"));
  _scheduler.worker_manager().addWorker ("worker_3", 1, capabilities ("worker_3", "C"));
  _scheduler.worker_manager().addWorker ("worker_4", 1, capabilities ("worker_4", "C"));
  _scheduler.worker_manager().addWorker ("worker_5", 1, capabilities ("worker_5", "C"));
  _scheduler.worker_manager().addWorker ("worker_6", 1, capabilities ("worker_6", "C"));
  _scheduler.worker_manager().addWorker ("worker_7", 1, capabilities ("worker_7", "C"));
  _scheduler.worker_manager().addWorker ("worker_8", 1, capabilities ("worker_8", "C"));

  _agent.TEST_add_dummy_job ("job_0", require ("C"));
  _agent.TEST_add_dummy_job ("job_1", require ("C"));
  _agent.TEST_add_dummy_job ("job_2", require ("C"));
  _agent.TEST_add_dummy_job ("job_3", require ("C"));
  _agent.TEST_add_dummy_job ("job_4", require ("C"));
  _agent.TEST_add_dummy_job ("job_5", require ("C"));
  _agent.TEST_add_dummy_job ("job_6", require ("C"));
  _agent.TEST_add_dummy_job ("job_7", require ("C"));
  _agent.TEST_add_dummy_job ("job_8", require ("C"));
  _agent.TEST_add_dummy_job ("job_9", require ("C"));
  _agent.TEST_add_dummy_job ("job_10", require ("C"));
  _agent.TEST_add_dummy_job ("job_11", require ("C"));
  _agent.TEST_add_dummy_job ("job_12", require ("C"));
  _agent.TEST_add_dummy_job ("job_13", require ("C"));
  _agent.TEST_add_dummy_job ("job_14", require ("C"));

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
  _scheduler.enqueueJob ("job_10");
  _scheduler.enqueueJob ("job_11");
  _scheduler.enqueueJob ("job_12");
  _scheduler.enqueueJob ("job_13");
  _scheduler.enqueueJob ("job_14");


  expect_serveJob_call ("job_0", worker_list ("worker_8"));
  expect_serveJob_call ("job_1", worker_list ("worker_7"));
  expect_serveJob_call ("job_2", worker_list ("worker_5"));
  expect_serveJob_call ("job_3", worker_list ("worker_4"));
  expect_serveJob_call ("job_4", worker_list ("worker_6"));
  expect_serveJob_call ("job_5", worker_list ("worker_3"));
  expect_serveJob_call ("job_6", worker_list ("worker_2"));
  expect_serveJob_call ("job_7", worker_list ("worker_1"));
  expect_serveJob_call ("job_8", worker_list ("worker_0"));

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().addWorker ("worker_9", 1, capabilities ("worker_9", "C"));

  expect_serveJob_call ("job_9", worker_list ("worker_9"));

  _scheduler.assignJobsToWorkers();
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerGainsCpbLater)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, capabilities ("worker_0", "C"));
  _scheduler.worker_manager().addWorker ("worker_1", 1, capabilities ("worker_1", "C"));
  _scheduler.worker_manager().addWorker ("worker_2", 1, capabilities ("worker_2", "C"));
  _scheduler.worker_manager().addWorker ("worker_3", 1, capabilities ("worker_3", "C"));
  _scheduler.worker_manager().addWorker ("worker_4", 1, capabilities ("worker_4", "C"));
  _scheduler.worker_manager().addWorker ("worker_5", 1, capabilities ("worker_5", "C"));
  _scheduler.worker_manager().addWorker ("worker_6", 1, capabilities ("worker_6", "C"));
  _scheduler.worker_manager().addWorker ("worker_7", 1, capabilities ("worker_7", "C"));
  _scheduler.worker_manager().addWorker ("worker_8", 1, capabilities ("worker_8", "C"));
  _scheduler.worker_manager().addWorker ("worker_9", 1);

  _agent.TEST_add_dummy_job ("job_0", require ("C"));
  _agent.TEST_add_dummy_job ("job_1", require ("C"));
  _agent.TEST_add_dummy_job ("job_2", require ("C"));
  _agent.TEST_add_dummy_job ("job_3", require ("C"));
  _agent.TEST_add_dummy_job ("job_4", require ("C"));
  _agent.TEST_add_dummy_job ("job_5", require ("C"));
  _agent.TEST_add_dummy_job ("job_6", require ("C"));
  _agent.TEST_add_dummy_job ("job_7", require ("C"));
  _agent.TEST_add_dummy_job ("job_8", require ("C"));
  _agent.TEST_add_dummy_job ("job_9", require ("C"));
  _agent.TEST_add_dummy_job ("job_10", require ("C"));
  _agent.TEST_add_dummy_job ("job_11", require ("C"));
  _agent.TEST_add_dummy_job ("job_12", require ("C"));
  _agent.TEST_add_dummy_job ("job_13", require ("C"));
  _agent.TEST_add_dummy_job ("job_14", require ("C"));

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
  _scheduler.enqueueJob ("job_10");
  _scheduler.enqueueJob ("job_11");
  _scheduler.enqueueJob ("job_12");
  _scheduler.enqueueJob ("job_13");
  _scheduler.enqueueJob ("job_14");


  expect_serveJob_call ("job_0", worker_list ("worker_8"));
  expect_serveJob_call ("job_1", worker_list ("worker_7"));
  expect_serveJob_call ("job_2", worker_list ("worker_5"));
  expect_serveJob_call ("job_3", worker_list ("worker_4"));
  expect_serveJob_call ("job_4", worker_list ("worker_6"));
  expect_serveJob_call ("job_5", worker_list ("worker_3"));
  expect_serveJob_call ("job_6", worker_list ("worker_2"));
  expect_serveJob_call ("job_7", worker_list ("worker_1"));
  expect_serveJob_call ("job_8", worker_list ("worker_0"));

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().findWorker ("worker_9")->addCapabilities (capabilities ("worker_9", "C"));

  expect_serveJob_call ("job_9", worker_list ("worker_9"));

  _scheduler.assignJobsToWorkers();
}

BOOST_AUTO_TEST_CASE(testCoallocSched)
{
  _scheduler.worker_manager().addWorker ("0", 1, capabilities ("0", "A"));
  _scheduler.worker_manager().addWorker ("1", 1, capabilities ("1", "B"));
  _scheduler.worker_manager().addWorker ("2", 1, capabilities ("2", "C"));
  _scheduler.worker_manager().addWorker ("3", 1, capabilities ("3", "A"));
  _scheduler.worker_manager().addWorker ("4", 1, capabilities ("4", "B"));
  _scheduler.worker_manager().addWorker ("5", 1, capabilities ("5", "C"));
  _scheduler.worker_manager().addWorker ("6", 1, capabilities ("6", "A"));
  _scheduler.worker_manager().addWorker ("7", 1, capabilities ("7", "B"));
  _scheduler.worker_manager().addWorker ("8", 1, capabilities ("8", "C"));
  _scheduler.worker_manager().addWorker ("9", 1, capabilities ("9", "A"));
  _scheduler.worker_manager().addWorker ("10", 1, capabilities ("10", "B"));
  _scheduler.worker_manager().addWorker ("11", 1, capabilities ("11", "C"));

  _agent.TEST_add_dummy_job ("job_0", require ("A", 4));
  _agent.TEST_add_dummy_job ("job_1", require ("B", 4));
  _agent.TEST_add_dummy_job ("job_2", require ("C", 4));

  _scheduler.enqueueJob ("job_0");
  _scheduler.enqueueJob ("job_1");
  _scheduler.enqueueJob ("job_2");


  expect_serveJob_call ("job_0", worker_list ("0", "3", "6", "9"));
  expect_serveJob_call ("job_1", worker_list ("1", "10", "4", "7"));
  expect_serveJob_call ("job_2", worker_list ("11", "2", "5", "8"));

  _scheduler.assignJobsToWorkers();


  _agent.TEST_add_dummy_job ("job_3", require ("A", 2));

  _scheduler.enqueueJob ("job_3");

  _scheduler.assignJobsToWorkers();

  _scheduler.worker_manager().findWorker ("0")->deleteJob ("job_0");
  _scheduler.worker_manager().findWorker ("3")->deleteJob ("job_0");
  _scheduler.worker_manager().findWorker ("6")->deleteJob ("job_0");
  _scheduler.worker_manager().findWorker ("9")->deleteJob ("job_0");

  _scheduler.releaseReservation("job_0");

  expect_serveJob_call ("job_3", 2, worker_list ("0", "3", "6", "9"));

  _scheduler.assignJobsToWorkers();
}

BOOST_AUTO_TEST_CASE(tesLBStopRestartWorker)
{
  _scheduler.worker_manager().addWorker ("worker_0", 1, capabilities ("worker_0", "C"));
  _scheduler.worker_manager().addWorker ("worker_1", 1, capabilities ("worker_1", "C"));
  _scheduler.worker_manager().addWorker ("worker_2", 1, capabilities ("worker_2", "C"));
  _scheduler.worker_manager().addWorker ("worker_3", 1, capabilities ("worker_3", "C"));
  _scheduler.worker_manager().addWorker ("worker_4", 1, capabilities ("worker_4", "C"));
  _scheduler.worker_manager().addWorker ("worker_5", 1, capabilities ("worker_5", "C"));
  _scheduler.worker_manager().addWorker ("worker_6", 1, capabilities ("worker_6", "C"));
  _scheduler.worker_manager().addWorker ("worker_7", 1, capabilities ("worker_7", "C"));
  _scheduler.worker_manager().addWorker ("worker_8", 1, capabilities ("worker_8", "C"));
  _scheduler.worker_manager().addWorker ("worker_9", 1, capabilities ("worker_9", "C"));

  _agent.TEST_add_dummy_job ("job_0", require ("C"));
  _agent.TEST_add_dummy_job ("job_1", require ("C"));
  _agent.TEST_add_dummy_job ("job_2", require ("C"));
  _agent.TEST_add_dummy_job ("job_3", require ("C"));
  _agent.TEST_add_dummy_job ("job_4", require ("C"));
  _agent.TEST_add_dummy_job ("job_5", require ("C"));
  _agent.TEST_add_dummy_job ("job_6", require ("C"));
  _agent.TEST_add_dummy_job ("job_7", require ("C"));
  _agent.TEST_add_dummy_job ("job_8", require ("C"));
  _agent.TEST_add_dummy_job ("job_9", require ("C"));

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


  expect_serveJob_call ("job_0", worker_list ("worker_9"));
  expect_serveJob_call ("job_1", worker_list ("worker_8"));
  expect_serveJob_call ("job_2", worker_list ("worker_7"));
  expect_serveJob_call ("job_3", worker_list ("worker_5"));
  expect_serveJob_call ("job_4", worker_list ("worker_4"));
  expect_serveJob_call ("job_5", worker_list ("worker_6"));
  expect_serveJob_call ("job_6", worker_list ("worker_3"));
  expect_serveJob_call ("job_7", worker_list ("worker_2"));
  expect_serveJob_call ("job_8", worker_list ("worker_1"));
  expect_serveJob_call ("job_9", worker_list ("worker_0"));

  _scheduler.assignJobsToWorkers();


  _scheduler.worker_manager().findWorker ("worker_9")->deleteJob ("job_0");
  _scheduler.releaseReservation ("job_0");
  _scheduler.worker_manager().deleteWorker ("worker_9");
  _scheduler.enqueueJob ("job_0");

  _scheduler.worker_manager().addWorker("worker_9", 1, capabilities ("worker_9", "C"));

  expect_serveJob_call ("job_0", worker_list ("worker_9"));

  _scheduler.assignJobsToWorkers();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_CASE
  (not_schedulable_job_does_not_block_others, allocate_test_agent_and_scheduler)
{
  _scheduler.worker_manager().addWorker ("worker", 1);

  _agent.TEST_add_dummy_job ("2", require (2));
  _scheduler.enqueueJob ("2");

  _scheduler.assignJobsToWorkers();

  _agent.TEST_add_dummy_job ("1", require (1));
  _scheduler.enqueueJob ("1");

  expect_serveJob_call ("1", worker_list ("worker"));
  _scheduler.assignJobsToWorkers();
}
