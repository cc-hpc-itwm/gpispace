#define BOOST_TEST_MODULE TestScheduler
#include <boost/test/unit_test.hpp>
#include <sdpa/daemon/agent/Agent.hpp>
#include "kvs_setup_fixture.hpp"

#include <fhg/util/boost/test/printer/list.hpp>
#include <fhg/util/boost/test/printer/set.hpp>

BOOST_TEST_DONT_PRINT_LOG_VALUE (sdpa::Capability)

using namespace std;
using namespace sdpa::daemon;

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

  ~TestAgent()
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

struct allocate_test_agent_and_scheduler
{
    allocate_test_agent_and_scheduler()
      : _agent ("agent", "127.0.0.1", kvs_host(), kvs_port(), sdpa::master_info_list_t())
      , _scheduler (&_agent)
    {}

    TestAgent _agent;
    sdpa::daemon::CoallocationScheduler _scheduler;
};

namespace
{
  sdpa::worker_id_list_t worker_list (sdpa::worker_id_t w1)
  {
    sdpa::worker_id_list_t list;
    list.push_back (w1);
    return list;
  }
  sdpa::worker_id_list_t worker_list ( sdpa::worker_id_t w1
                                     , sdpa::worker_id_t w2
                                     )
  {
    sdpa::worker_id_list_t list;
    list.push_back (w1);
    list.push_back (w2);
    return list;
  }
  sdpa::worker_id_list_t worker_list ( sdpa::worker_id_t w1
                                     , sdpa::worker_id_t w2
                                     , sdpa::worker_id_t w3
                                     , sdpa::worker_id_t w4
                                     )
  {
    sdpa::worker_id_list_t list;
    list.push_back (w1);
    list.push_back (w2);
    list.push_back (w3);
    list.push_back (w4);
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
}

BOOST_FIXTURE_TEST_SUITE( test_Scheduler, allocate_test_agent_and_scheduler)

BOOST_GLOBAL_FIXTURE (KVSSetup)

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
  _scheduler.schedule ("job_10");
  _scheduler.schedule ("job_11");
  _scheduler.schedule ("job_12");
  _scheduler.schedule ("job_13");
  _scheduler.schedule ("job_14");


  _agent.expect_serveJob_call ("job_0", worker_list ("worker_9"));
  _agent.expect_serveJob_call ("job_1", worker_list ("worker_8"));
  _agent.expect_serveJob_call ("job_2", worker_list ("worker_7"));
  _agent.expect_serveJob_call ("job_3", worker_list ("worker_5"));
  _agent.expect_serveJob_call ("job_4", worker_list ("worker_4"));
  _agent.expect_serveJob_call ("job_5", worker_list ("worker_6"));
  _agent.expect_serveJob_call ("job_6", worker_list ("worker_3"));
  _agent.expect_serveJob_call ("job_7", worker_list ("worker_2"));
  _agent.expect_serveJob_call ("job_8", worker_list ("worker_1"));
  _agent.expect_serveJob_call ("job_9", worker_list ("worker_0"));

  _scheduler.assignJobsToWorkers();


  _scheduler.deleteWorkerJob ("worker_9", "job_0");
  _scheduler.deleteWorkerJob ("worker_8", "job_1");
  _scheduler.deleteWorkerJob ("worker_7", "job_2");
  _scheduler.deleteWorkerJob ("worker_5", "job_3");
  _scheduler.deleteWorkerJob ("worker_4", "job_4");
  _scheduler.deleteWorkerJob ("worker_6", "job_5");
  _scheduler.deleteWorkerJob ("worker_3", "job_6");
  _scheduler.deleteWorkerJob ("worker_2", "job_7");
  _scheduler.deleteWorkerJob ("worker_1", "job_8");
  _scheduler.deleteWorkerJob ("worker_0", "job_9");

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


  _agent.expect_serveJob_call ("job_10", worker_list ("worker_9"));
  _agent.expect_serveJob_call ("job_11", worker_list ("worker_8"));
  _agent.expect_serveJob_call ("job_12", worker_list ("worker_7"));
  _agent.expect_serveJob_call ("job_13", worker_list ("worker_5"));
  _agent.expect_serveJob_call ("job_14", worker_list ("worker_4"));

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
  _scheduler.schedule ("job_10");
  _scheduler.schedule ("job_11");
  _scheduler.schedule ("job_12");
  _scheduler.schedule ("job_13");
  _scheduler.schedule ("job_14");


  _agent.expect_serveJob_call ("job_0", worker_list ("worker_8"));
  _agent.expect_serveJob_call ("job_1", worker_list ("worker_7"));
  _agent.expect_serveJob_call ("job_2", worker_list ("worker_5"));
  _agent.expect_serveJob_call ("job_3", worker_list ("worker_4"));
  _agent.expect_serveJob_call ("job_4", worker_list ("worker_6"));
  _agent.expect_serveJob_call ("job_5", worker_list ("worker_3"));
  _agent.expect_serveJob_call ("job_6", worker_list ("worker_2"));
  _agent.expect_serveJob_call ("job_7", worker_list ("worker_1"));
  _agent.expect_serveJob_call ("job_8", worker_list ("worker_0"));

  _scheduler.assignJobsToWorkers();


  _scheduler.addWorker ("worker_9", 1, capabilities ("worker_9", "C"));

  _agent.expect_serveJob_call ("job_9", worker_list ("worker_9"));

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
  _scheduler.schedule ("job_10");
  _scheduler.schedule ("job_11");
  _scheduler.schedule ("job_12");
  _scheduler.schedule ("job_13");
  _scheduler.schedule ("job_14");


  _agent.expect_serveJob_call ("job_0", worker_list ("worker_8"));
  _agent.expect_serveJob_call ("job_1", worker_list ("worker_7"));
  _agent.expect_serveJob_call ("job_2", worker_list ("worker_5"));
  _agent.expect_serveJob_call ("job_3", worker_list ("worker_4"));
  _agent.expect_serveJob_call ("job_4", worker_list ("worker_6"));
  _agent.expect_serveJob_call ("job_5", worker_list ("worker_3"));
  _agent.expect_serveJob_call ("job_6", worker_list ("worker_2"));
  _agent.expect_serveJob_call ("job_7", worker_list ("worker_1"));
  _agent.expect_serveJob_call ("job_8", worker_list ("worker_0"));

  _scheduler.assignJobsToWorkers();


  _scheduler.addCapabilities ("worker_9", capabilities ("worker_9", "C"));

  _agent.expect_serveJob_call ("job_9", worker_list ("worker_9"));

  _scheduler.assignJobsToWorkers();
}

BOOST_AUTO_TEST_CASE(testCoallocSched)
{
  _scheduler.addWorker ("0", 1, capabilities ("0", "A"));
  _scheduler.addWorker ("1", 1, capabilities ("1", "B"));
  _scheduler.addWorker ("2", 1, capabilities ("2", "C"));
  _scheduler.addWorker ("3", 1, capabilities ("3", "A"));
  _scheduler.addWorker ("4", 1, capabilities ("4", "B"));
  _scheduler.addWorker ("5", 1, capabilities ("5", "C"));
  _scheduler.addWorker ("6", 1, capabilities ("6", "A"));
  _scheduler.addWorker ("7", 1, capabilities ("7", "B"));
  _scheduler.addWorker ("8", 1, capabilities ("8", "C"));
  _scheduler.addWorker ("9", 1, capabilities ("9", "A"));
  _scheduler.addWorker ("10", 1, capabilities ("10", "B"));
  _scheduler.addWorker ("11", 1, capabilities ("11", "C"));

  _agent.TEST_add_dummy_job ("job_0", require ("A", 4));
  _agent.TEST_add_dummy_job ("job_1", require ("B", 4));
  _agent.TEST_add_dummy_job ("job_2", require ("C", 4));

  _scheduler.schedule("job_0");
  _scheduler.schedule("job_1");
  _scheduler.schedule("job_2");


  _agent.expect_serveJob_call ("job_0", worker_list ("6", "3", "9", "0"));
  _agent.expect_serveJob_call ("job_1", worker_list ("10", "7", "4", "1"));
  _agent.expect_serveJob_call ("job_2", worker_list ("8", "11", "2", "5"));

  _scheduler.assignJobsToWorkers();


  _agent.TEST_add_dummy_job ("job_3", require ("A", 2));

  _scheduler.schedule("job_3");

  _scheduler.assignJobsToWorkers();


  _scheduler.releaseReservation("job_0");

  _agent.expect_serveJob_call ("job_3", worker_list ("6", "3"));

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


  _agent.expect_serveJob_call ("job_0", worker_list ("worker_9"));
  _agent.expect_serveJob_call ("job_1", worker_list ("worker_8"));
  _agent.expect_serveJob_call ("job_2", worker_list ("worker_7"));
  _agent.expect_serveJob_call ("job_3", worker_list ("worker_5"));
  _agent.expect_serveJob_call ("job_4", worker_list ("worker_4"));
  _agent.expect_serveJob_call ("job_5", worker_list ("worker_6"));
  _agent.expect_serveJob_call ("job_6", worker_list ("worker_3"));
  _agent.expect_serveJob_call ("job_7", worker_list ("worker_2"));
  _agent.expect_serveJob_call ("job_8", worker_list ("worker_1"));
  _agent.expect_serveJob_call ("job_9", worker_list ("worker_0"));

  _scheduler.assignJobsToWorkers();


  _scheduler.deleteWorker("worker_9");
  _scheduler.addWorker("worker_9", 1, capabilities ("worker_9", "C"));

  _agent.expect_serveJob_call ("job_0", worker_list ("worker_9"));

  _scheduler.assignJobsToWorkers();
}

BOOST_AUTO_TEST_SUITE_END()
