#define BOOST_TEST_MODULE TestSimpleScheduler
#include <boost/test/unit_test.hpp>
#include <sdpa/daemon/scheduler/SimpleScheduler.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include "kvs_setup_fixture.hpp"

#include <fhg/util/boost/test/printer/list.hpp>
#include <fhg/util/boost/test/printer/set.hpp>

BOOST_TEST_DONT_PRINT_LOG_VALUE (sdpa::Capability)

using namespace std;
using namespace sdpa::daemon;

const std::string WORKER_CPBS[] = {"A", "B", "C"};

typedef std::map<sdpa::job_id_t, sdpa::worker_id_t> mapJob2Worker_t;

class TestOrchestrator : public sdpa::daemon::Orchestrator
{
public:
  typedef boost::shared_ptr<TestOrchestrator > ptr_t;
  TestOrchestrator( const std::string& name, const std::string& url, std::string kvs_host, std::string kvs_port)
    : sdpa::daemon::Orchestrator(name, url, kvs_host, kvs_port)
  {
  }

  ~TestOrchestrator()
  {
    BOOST_REQUIRE (_expected_serveJob_calls.empty());
  }

  void serveJob(const sdpa::worker_id_list_t& worker_list, const sdpa::job_id_t& jobId)
  {
    BOOST_REQUIRE_GE (_expected_serveJob_calls.count (jobId), 1);
    BOOST_CHECK_EQUAL (_expected_serveJob_calls[jobId], worker_list);

    _expected_serveJob_calls.erase (_expected_serveJob_calls.find (jobId));
  }

  void submitWorkflow(const we::layer::id_type&)
  {
    throw std::runtime_error ("trying to submit workflow in test casse which never should");
  }

  void sendEventToOther(const sdpa::events::SDPAEvent::Ptr&)
  {
    throw std::runtime_error ("trying to send message in test case which should not send messages");
  }

  void expect_serveJob_call (sdpa::job_id_t id, sdpa::worker_id_list_t list)
  {
    _expected_serveJob_calls.insert (std::make_pair (id, list));
  }

  std::map<sdpa::job_id_t, sdpa::worker_id_list_t> _expected_serveJob_calls;
};

struct allocate_test_orchestrator_and_scheduler
{
    allocate_test_orchestrator_and_scheduler()
      : _orchestrator ("orchestrator", "127.0.0.1", kvs_host(), kvs_port())
      , _scheduler (&_orchestrator)
    {}

    TestOrchestrator _orchestrator;
    sdpa::daemon::SimpleScheduler _scheduler;
};

namespace
{
  sdpa::worker_id_list_t worker_list (sdpa::worker_id_t w1)
  {
    sdpa::worker_id_list_t list;
    list.push_back (w1);
    return list;
  }

  sdpa::capabilities_set_t capabilities ( sdpa::worker_id_t worker
                                        , std::string name_1
                                        )
  {
    sdpa::capabilities_set_t set;
    set.insert (sdpa::capability_t (name_1, worker));
    return set;
  }
  sdpa::capabilities_set_t capabilities ( sdpa::worker_id_t worker
                                        , std::string name_1
                                        , std::string name_2
                                        , std::string name_3
                                        )
  {
    sdpa::capabilities_set_t set;
    set.insert (sdpa::capability_t (name_1, worker));
    set.insert (sdpa::capability_t (name_2, worker));
    set.insert (sdpa::capability_t (name_3, worker));
    return set;
  }

  job_requirements_t require (std::string name_1)
  {
    requirement_list_t reqs;
    reqs.push_back (we::type::requirement_t (name_1, true));
    return job_requirements_t (reqs, we::type::schedule_data());
  }
  job_requirements_t require (std::string name_1, std::string name_2)
  {
    requirement_list_t reqs;
    reqs.push_back (we::type::requirement_t (name_1, true));
    reqs.push_back (we::type::requirement_t (name_2, true));
    return job_requirements_t (reqs, we::type::schedule_data());
  }
}

BOOST_FIXTURE_TEST_SUITE( test_Scheduler, allocate_test_orchestrator_and_scheduler)

BOOST_GLOBAL_FIXTURE (KVSSetup)

//! \note Actually a test for SchedulerBase
BOOST_AUTO_TEST_CASE(testCapabilitiesMatching)
{
  const sdpa::worker_id_t workerId ("test_worker");
  const sdpa::capabilities_set_t workerCpbSet
    (capabilities (workerId, "A", "B", "C"));

  _scheduler.addWorker(workerId, 1, workerCpbSet);

  BOOST_REQUIRE_EQUAL (workerCpbSet, _scheduler.getWorkerCapabilities(workerId));

  BOOST_REQUIRE_EQUAL ( workerId
                      , _scheduler.findSuitableWorker
                        (require ("A", "B"), worker_list (workerId))
                      );
}

//! \note Actually a test for SchedulerBase
BOOST_AUTO_TEST_CASE(testGainCap)
{
  const sdpa::worker_id_t worker_A ("worker_A");

  _scheduler.addWorker(worker_A, 1, sdpa::capabilities_set_t());

  const sdpa::job_id_t jobId1("Job1");
  _orchestrator.TEST_add_dummy_job (jobId1, require ("C"));

  _scheduler.schedule(jobId1);

  _scheduler.assignJobsToWorkers();

  const sdpa::capabilities_set_t cpbSetA (capabilities (worker_A, "C"));
  _scheduler.addCapabilities (worker_A, cpbSetA);

  BOOST_REQUIRE_EQUAL (cpbSetA, _scheduler.getWorkerCapabilities(worker_A));

  _orchestrator.expect_serveJob_call (jobId1, worker_list (worker_A));

  _scheduler.assignJobsToWorkers();
}


BOOST_AUTO_TEST_CASE(testLoadBalancing)
{
  _scheduler.addWorker ("worker_0", 1, capabilities ("worker_0", "C"));
  _scheduler.addWorker ("worker_1", 1, capabilities ("worker_1", "C"));
  _scheduler.addWorker ("worker_2", 1, capabilities ("worker_2", "C"));
  _scheduler.addWorker ("worker_3", 1, capabilities ("worker_3", "C"));
  _scheduler.addWorker ("worker_4", 1, capabilities ("worker_4", "C"));
  _scheduler.addWorker ("worker_5", 1, capabilities ("worker_5", "C"));
  _scheduler.addWorker ("worker_6", 1, capabilities ("worker_6", "C"));
  _scheduler.addWorker ("worker_7", 1, capabilities ("worker_7", "C"));
  _scheduler.addWorker ("worker_8", 1, capabilities ("worker_8", "C"));
  _scheduler.addWorker ("worker_9", 1, capabilities ("worker_9", "C"));

  _orchestrator.TEST_add_dummy_job ("job_0", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_1", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_2", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_3", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_4", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_5", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_6", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_7", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_8", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_9", require ("C"));

  _scheduler.schedule ("job_0");
  _scheduler.schedule ("job_1");
  _scheduler.schedule ("job_2");
  _scheduler.schedule ("job_3");
  _scheduler.schedule ("job_4");
  _scheduler.schedule ("job_5");
  _scheduler.schedule ("job_6");
  _scheduler.schedule ("job_7");
  _scheduler.schedule ("job_8");
  _scheduler.schedule ("job_9");


  _orchestrator.expect_serveJob_call ("job_0", worker_list ("worker_9"));
  _orchestrator.expect_serveJob_call ("job_1", worker_list ("worker_8"));
  _orchestrator.expect_serveJob_call ("job_2", worker_list ("worker_7"));
  _orchestrator.expect_serveJob_call ("job_3", worker_list ("worker_5"));
  _orchestrator.expect_serveJob_call ("job_4", worker_list ("worker_4"));
  _orchestrator.expect_serveJob_call ("job_5", worker_list ("worker_6"));
  _orchestrator.expect_serveJob_call ("job_6", worker_list ("worker_3"));
  _orchestrator.expect_serveJob_call ("job_7", worker_list ("worker_2"));
  _orchestrator.expect_serveJob_call ("job_8", worker_list ("worker_1"));
  _orchestrator.expect_serveJob_call ("job_9", worker_list ("worker_0"));

  _scheduler.assignJobsToWorkers();
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerJoinsLater)
{
  _scheduler.addWorker ("worker_0", 1, capabilities ("worker_0", "C"));
  _scheduler.addWorker ("worker_1", 1, capabilities ("worker_1", "C"));
  _scheduler.addWorker ("worker_2", 1, capabilities ("worker_2", "C"));
  _scheduler.addWorker ("worker_3", 1, capabilities ("worker_3", "C"));
  _scheduler.addWorker ("worker_4", 1, capabilities ("worker_4", "C"));
  _scheduler.addWorker ("worker_5", 1, capabilities ("worker_5", "C"));
  _scheduler.addWorker ("worker_6", 1, capabilities ("worker_6", "C"));
  _scheduler.addWorker ("worker_7", 1, capabilities ("worker_7", "C"));
  _scheduler.addWorker ("worker_8", 1, capabilities ("worker_8", "C"));

  _orchestrator.TEST_add_dummy_job ("job_0", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_1", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_2", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_3", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_4", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_5", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_6", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_7", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_8", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_9", require ("C"));

  _scheduler.schedule ("job_0");
  _scheduler.schedule ("job_1");
  _scheduler.schedule ("job_2");
  _scheduler.schedule ("job_3");
  _scheduler.schedule ("job_4");
  _scheduler.schedule ("job_5");
  _scheduler.schedule ("job_6");
  _scheduler.schedule ("job_7");
  _scheduler.schedule ("job_8");
  _scheduler.schedule ("job_9");


  _orchestrator.expect_serveJob_call ("job_0", worker_list ("worker_8"));
  _orchestrator.expect_serveJob_call ("job_1", worker_list ("worker_7"));
  _orchestrator.expect_serveJob_call ("job_2", worker_list ("worker_5"));
  _orchestrator.expect_serveJob_call ("job_3", worker_list ("worker_4"));
  _orchestrator.expect_serveJob_call ("job_4", worker_list ("worker_6"));
  _orchestrator.expect_serveJob_call ("job_5", worker_list ("worker_3"));
  _orchestrator.expect_serveJob_call ("job_6", worker_list ("worker_2"));
  _orchestrator.expect_serveJob_call ("job_7", worker_list ("worker_1"));
  _orchestrator.expect_serveJob_call ("job_8", worker_list ("worker_0"));

  _scheduler.assignJobsToWorkers();


  _scheduler.addWorker ("worker_9", 1, capabilities ("worker_9", "C"));

  _orchestrator.expect_serveJob_call ("job_9", worker_list ("worker_9"));

  _scheduler.assignJobsToWorkers();
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerGainsCpbLater)
{
  _scheduler.addWorker ("worker_0", 1, capabilities ("worker_0", "C"));
  _scheduler.addWorker ("worker_1", 1, capabilities ("worker_1", "C"));
  _scheduler.addWorker ("worker_2", 1, capabilities ("worker_2", "C"));
  _scheduler.addWorker ("worker_3", 1, capabilities ("worker_3", "C"));
  _scheduler.addWorker ("worker_4", 1, capabilities ("worker_4", "C"));
  _scheduler.addWorker ("worker_5", 1, capabilities ("worker_5", "C"));
  _scheduler.addWorker ("worker_6", 1, capabilities ("worker_6", "C"));
  _scheduler.addWorker ("worker_7", 1, capabilities ("worker_7", "C"));
  _scheduler.addWorker ("worker_8", 1, capabilities ("worker_8", "C"));
  _scheduler.addWorker ("worker_9", 1);

  _orchestrator.TEST_add_dummy_job ("job_0", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_1", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_2", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_3", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_4", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_5", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_6", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_7", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_8", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_9", require ("C"));

  _scheduler.schedule ("job_0");
  _scheduler.schedule ("job_1");
  _scheduler.schedule ("job_2");
  _scheduler.schedule ("job_3");
  _scheduler.schedule ("job_4");
  _scheduler.schedule ("job_5");
  _scheduler.schedule ("job_6");
  _scheduler.schedule ("job_7");
  _scheduler.schedule ("job_8");
  _scheduler.schedule ("job_9");


  _orchestrator.expect_serveJob_call ("job_0", worker_list ("worker_8"));
  _orchestrator.expect_serveJob_call ("job_1", worker_list ("worker_7"));
  _orchestrator.expect_serveJob_call ("job_2", worker_list ("worker_5"));
  _orchestrator.expect_serveJob_call ("job_3", worker_list ("worker_4"));
  _orchestrator.expect_serveJob_call ("job_4", worker_list ("worker_6"));
  _orchestrator.expect_serveJob_call ("job_5", worker_list ("worker_3"));
  _orchestrator.expect_serveJob_call ("job_6", worker_list ("worker_2"));
  _orchestrator.expect_serveJob_call ("job_7", worker_list ("worker_1"));
  _orchestrator.expect_serveJob_call ("job_8", worker_list ("worker_0"));

  _scheduler.assignJobsToWorkers();


  _scheduler.addCapabilities ("worker_9", capabilities ("worker_9", "C"));

  _orchestrator.expect_serveJob_call ("job_9", worker_list ("worker_9"));

  _scheduler.assignJobsToWorkers();
}

BOOST_AUTO_TEST_CASE(tesLBStopRestartWorker)
{
  _scheduler.addWorker ("worker_0", 1, capabilities ("worker_0", "C"));
  _scheduler.addWorker ("worker_1", 1, capabilities ("worker_1", "C"));
  _scheduler.addWorker ("worker_2", 1, capabilities ("worker_2", "C"));
  _scheduler.addWorker ("worker_3", 1, capabilities ("worker_3", "C"));
  _scheduler.addWorker ("worker_4", 1, capabilities ("worker_4", "C"));
  _scheduler.addWorker ("worker_5", 1, capabilities ("worker_5", "C"));
  _scheduler.addWorker ("worker_6", 1, capabilities ("worker_6", "C"));
  _scheduler.addWorker ("worker_7", 1, capabilities ("worker_7", "C"));
  _scheduler.addWorker ("worker_8", 1, capabilities ("worker_8", "C"));
  _scheduler.addWorker ("worker_9", 1, capabilities ("worker_9", "C"));

  _orchestrator.TEST_add_dummy_job ("job_0", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_1", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_2", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_3", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_4", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_5", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_6", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_7", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_8", require ("C"));
  _orchestrator.TEST_add_dummy_job ("job_9", require ("C"));

  _scheduler.schedule ("job_0");
  _scheduler.schedule ("job_1");
  _scheduler.schedule ("job_2");
  _scheduler.schedule ("job_3");
  _scheduler.schedule ("job_4");
  _scheduler.schedule ("job_5");
  _scheduler.schedule ("job_6");
  _scheduler.schedule ("job_7");
  _scheduler.schedule ("job_8");
  _scheduler.schedule ("job_9");


  _orchestrator.expect_serveJob_call ("job_0", worker_list ("worker_9"));
  _orchestrator.expect_serveJob_call ("job_1", worker_list ("worker_8"));
  _orchestrator.expect_serveJob_call ("job_2", worker_list ("worker_7"));
  _orchestrator.expect_serveJob_call ("job_3", worker_list ("worker_5"));
  _orchestrator.expect_serveJob_call ("job_4", worker_list ("worker_4"));
  _orchestrator.expect_serveJob_call ("job_5", worker_list ("worker_6"));
  _orchestrator.expect_serveJob_call ("job_6", worker_list ("worker_3"));
  _orchestrator.expect_serveJob_call ("job_7", worker_list ("worker_2"));
  _orchestrator.expect_serveJob_call ("job_8", worker_list ("worker_1"));
  _orchestrator.expect_serveJob_call ("job_9", worker_list ("worker_0"));

  _scheduler.assignJobsToWorkers();


  _scheduler.deleteWorker ("worker_9");
  _scheduler.addWorker ("worker_9", 1, capabilities ("worker_9", "C"));

  _orchestrator.expect_serveJob_call ("job_0", worker_list ("worker_9"));

  _scheduler.assignJobsToWorkers();
}

BOOST_AUTO_TEST_SUITE_END()
