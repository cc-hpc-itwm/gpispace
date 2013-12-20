/*
 * =====================================================================================
 *
 *       Filename:  test_CoallocationScheduler.cpp
 *
 *    Description:  test the scheduler thread
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#define BOOST_TEST_MODULE TestScheduler
#include <boost/test/unit_test.hpp>
#include <sdpa/daemon/agent/Agent.hpp>
#include "kvs_setup_fixture.hpp"

#include <fhg/util/boost/test/printer/list.hpp>
#include <fhg/util/boost/test/printer/set.hpp>

BOOST_TEST_DONT_PRINT_LOG_VALUE (sdpa::Capability)
BOOST_TEST_DONT_PRINT_LOG_VALUE (sdpa::job_id_t)

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
             , const sdpa::master_info_list_t& arrMasterNames
             , const unsigned int rank = 0
             , const boost::optional<std::string>& appGuiUrl = boost::none)
    : sdpa::daemon::Agent(name, url, arrMasterNames, rank, appGuiUrl)
  {
  }

  ~TestAgent()
  {
    BOOST_REQUIRE (_expected_serveJob_calls.empty());
  }

  void serveJob(const sdpa::worker_id_t& wid, const sdpa::job_id_t& jobId)
  {
    throw std::runtime_error ("scheduled job to only one worker, not many");
  }


  void serveJob(const sdpa::worker_id_list_t& worker_list, const sdpa::job_id_t& jobId)
  {
    BOOST_REQUIRE_GE (_expected_serveJob_calls.count (jobId), 1);
    BOOST_CHECK_EQUAL (_expected_serveJob_calls[jobId], worker_list);

    _expected_serveJob_calls.erase (_expected_serveJob_calls.find (jobId));
  }

  void submitWorkflow(const we::mgmt::layer::id_type& id, const we::mgmt::layer::encoded_type& )
  {
    DLOG(TRACE, "The agent is trying to forward the master job "<<id<<" to the workflow engine");
    throw std::runtime_error ("trying to submit workflow in test casse which never should");
  }

  void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& pEvt)
  {
    DLOG(TRACE, "The agent is trying to send a message of type "<<pEvt->str()<<" to the daemon stage");
    throw std::runtime_error ("trying to send message in test case which should not send messages");
  }

  void sendEventToOther(const sdpa::events::SDPAEvent::Ptr& pEvt)
  {
    DLOG(TRACE, "The agent is trying to send a message of type "<<pEvt->str()<<" to the master stage");
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
      : _agent ("agent", "127.0.0.1", sdpa::master_info_list_t())
      , _scheduler (&_agent)
    {}

    TestAgent _agent;
    sdpa::daemon::CoallocationScheduler _scheduler;
};

namespace
{
  sdpa::worker_id_list_t make_list (sdpa::worker_id_t w1)
  {
    sdpa::worker_id_list_t list;
    list.push_back (w1);
    return list;
  }
  sdpa::worker_id_list_t make_list ( sdpa::worker_id_t w1
                                   , sdpa::worker_id_t w2
                                   )
  {
    sdpa::worker_id_list_t list;
    list.push_back (w1);
    list.push_back (w2);
    return list;
  }
  sdpa::worker_id_list_t make_list ( sdpa::worker_id_t w1
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
}

BOOST_FIXTURE_TEST_SUITE( test_Scheduler, allocate_test_agent_and_scheduler)

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE(testCapabilitiesMatching)
{
  LOG( INFO, "Test if the capabilities are matching the requirements "<<std::endl);

  sdpa::worker_id_t workerId("test_worker");
  sdpa::capabilities_set_t workerCpbSet;

  workerCpbSet.insert(sdpa::capability_t("A","",workerId));
  workerCpbSet.insert(sdpa::capability_t("B","",workerId));
  workerCpbSet.insert(sdpa::capability_t("C","",workerId));

  _scheduler.addWorker(workerId, 1, workerCpbSet);

  // check what are the agent's capabilites now
  sdpa::capabilities_set_t acquiredCpbs;
  _scheduler.getWorkerCapabilities(workerId, acquiredCpbs);

  BOOST_REQUIRE_EQUAL (workerCpbSet, acquiredCpbs);

  // Now, create a job that requires the capabilities A and B
  requirement_list_t reqList;
  reqList.push_back(we::type::requirement_t("A", true));
  reqList.push_back(we::type::requirement_t("B", true));
  job_requirements_t jobReqs(reqList, we::type::schedule_data());

  // check if there is any matching worker
  sdpa::worker_id_list_t avail (1, workerId);
  BOOST_REQUIRE_EQUAL (workerId, _scheduler.findSuitableWorker (jobReqs, avail));
}

BOOST_AUTO_TEST_CASE(testGainCap)
{
  LOG(INFO, "Test scheduling when the required capabilities are gained later ...");

  sdpa::worker_id_t worker_A("worker_A");

  sdpa::capabilities_set_t cpbSetA;
  _scheduler.addWorker(worker_A, 1, cpbSetA);

  const sdpa::job_id_t jobId1("Job1");
  job_requirements_t jobReqs1(requirement_list_t(1, we::type::requirement_t("C", true)), we::type::schedule_data(1, 100));
  _agent.addJob(jobId1, "description 1", sdpa::job_id_t(), false, "", jobReqs1);

  LOG(DEBUG, "Schedule the job "<<jobId1);
  _scheduler.schedule(jobId1);

  _scheduler.assignJobsToWorkers(); _scheduler.checkAllocations();

  BOOST_REQUIRE (_scheduler.getListAllocatedWorkers (jobId1).empty());

  sdpa::capability_t cpb1("C", "virtual", worker_A);
  cpbSetA.insert(cpb1);
  _scheduler.addCapabilities(worker_A, cpbSetA);

  LOG(DEBUG, "Check if worker_A really acquired the capability \"C\"");

  sdpa::capabilities_set_t cpbset;
  _scheduler.getWorkerCapabilities(worker_A, cpbset);

  LOG(DEBUG, "The worker_A has now the following capabilities: ["<<cpbset<<"]");

  _agent.expect_serveJob_call (jobId1, make_list (worker_A));

  LOG(DEBUG, "Try to assign again jobs to the workers ...");
  _scheduler.assignJobsToWorkers();
  _scheduler.checkAllocations();

  BOOST_REQUIRE_EQUAL ( sdpa::worker_id_list_t (1, worker_A)
                      , _scheduler.getListAllocatedWorkers (jobId1)
                      );
}

BOOST_AUTO_TEST_CASE(testLoadBalancing)
{
  LOG(INFO, "testLoadBalancing");

  // number of workers
  const int nWorkers = 10;
  const int nJobs = 15;

  // create a give number of workers with different capabilities:
  std::ostringstream osstr;
  std::vector<sdpa::worker_id_t> arrWorkerIds;
  for(int k=0;k<nWorkers;k++)
  {
      osstr<<"worker_"<<k;
      sdpa::worker_id_t workerId(osstr.str());
      osstr.str("");
      arrWorkerIds.push_back(workerId);
      std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", workerId));
      sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
      _scheduler.addWorker(workerId, 1, cpbSet);
  }

  // submit a bunch of jobs now
  std::vector<sdpa::job_id_t> arrJobIds;
  for(int i=0;i<nJobs;i++)
  {
      osstr<<"job_"<<i;
      sdpa::job_id_t jobId(osstr.str());
      arrJobIds.push_back(jobId);
      osstr.str("");
      job_requirements_t job_reqs(requirement_list_t(1, we::type::requirement_t("C", true)), we::type::schedule_data(1, 100));
      _agent.addJob(jobId, "", sdpa::job_id_t(), false, "", job_reqs);
  }

  _agent.expect_serveJob_call ("job_0", make_list ("worker_9"));
  _agent.expect_serveJob_call ("job_1", make_list ("worker_8"));
  _agent.expect_serveJob_call ("job_2", make_list ("worker_7"));
  _agent.expect_serveJob_call ("job_3", make_list ("worker_5"));
  _agent.expect_serveJob_call ("job_4", make_list ("worker_4"));
  _agent.expect_serveJob_call ("job_5", make_list ("worker_6"));
  _agent.expect_serveJob_call ("job_6", make_list ("worker_3"));
  _agent.expect_serveJob_call ("job_7", make_list ("worker_2"));
  _agent.expect_serveJob_call ("job_8", make_list ("worker_1"));
  _agent.expect_serveJob_call ("job_9", make_list ("worker_0"));
  _agent.expect_serveJob_call ("job_10", make_list ("worker_9"));
  _agent.expect_serveJob_call ("job_11", make_list ("worker_8"));
  _agent.expect_serveJob_call ("job_12", make_list ("worker_7"));
  _agent.expect_serveJob_call ("job_13", make_list ("worker_5"));
  _agent.expect_serveJob_call ("job_14", make_list ("worker_4"));

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
  {
      _scheduler.schedule(jobId);
  }

  _scheduler.assignJobsToWorkers(); _scheduler.checkAllocations();

  sdpa::worker_id_list_t workerList;
  _scheduler.getListNotAllocatedWorkers(workerList);

  // check if there are any workers that are not yet reserved
  BOOST_CHECK(workerList.empty());

  BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
  {
      sdpa::worker_id_list_t listJobAssignedWorkers = _scheduler.getListAllocatedWorkers(jobId);

      BOOST_CHECK_LE (listJobAssignedWorkers.size(), 1);
      if(!listJobAssignedWorkers.empty())
      {
          // delete the job
          _scheduler.deleteWorkerJob(listJobAssignedWorkers.front(), jobId);
      }

      _scheduler.releaseReservation(jobId);
  }

  _scheduler.assignJobsToWorkers();
  _scheduler.checkAllocations();

  workerList.clear();
  _scheduler.getListNotAllocatedWorkers(workerList);

  BOOST_CHECK_EQUAL(workerList.size(), nJobs - nWorkers);
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerJoinsLater)
{
  LOG(INFO, "Test the load-balancing when a worker joins later ...");

  // number of workers
  const int nWorkers = 10;
  const int nJobs = 15;

  // create a give number of workers with different capabilities:
  std::ostringstream osstr;
  std::vector<sdpa::worker_id_t> arrWorkerIds;
  for(int k=0;k<nWorkers-1;k++)
  {
      osstr<<"worker_"<<k;
      sdpa::worker_id_t workerId(osstr.str());
      osstr.str("");
      arrWorkerIds.push_back(workerId);
      std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", workerId));
      sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
      _scheduler.addWorker(workerId, 1, cpbSet);
  }

  // submit a bunch of jobs now
  std::vector<sdpa::job_id_t> arrJobIds;
  for(int i=0;i<nJobs;i++)
  {
      osstr<<"job_"<<i;
      sdpa::job_id_t jobId(osstr.str());
      arrJobIds.push_back(jobId);
      osstr.str("");
      job_requirements_t job_reqs(requirement_list_t(1, we::type::requirement_t("C", true)), we::type::schedule_data(1, 100));
      _agent.addJob(jobId, "", sdpa::job_id_t(), false, "", job_reqs);
  }

  _agent.expect_serveJob_call ("job_0", make_list ("worker_8"));
  _agent.expect_serveJob_call ("job_1", make_list ("worker_7"));
  _agent.expect_serveJob_call ("job_2", make_list ("worker_5"));
  _agent.expect_serveJob_call ("job_3", make_list ("worker_4"));
  _agent.expect_serveJob_call ("job_4", make_list ("worker_6"));
  _agent.expect_serveJob_call ("job_5", make_list ("worker_3"));
  _agent.expect_serveJob_call ("job_6", make_list ("worker_2"));
  _agent.expect_serveJob_call ("job_7", make_list ("worker_1"));
  _agent.expect_serveJob_call ("job_8", make_list ("worker_0"));

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
  {
      _scheduler.schedule(jobId);
  }

  _scheduler.assignJobsToWorkers(); _scheduler.checkAllocations();

  // all the workers should have assigned jobs
  sdpa::worker_id_list_t workerList;
  _scheduler.getListNotAllocatedWorkers(workerList);
  // check if there are any workers that are not yet reserved
  BOOST_CHECK(workerList.empty());

  // add new worker now (worker_9)...
  osstr<<"worker_"<<nWorkers-1;
  sdpa::worker_id_t workerId(osstr.str());
  osstr.str("");
  arrWorkerIds.push_back(workerId);
  std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", workerId));
  sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
  _scheduler.addWorker(workerId, 1, cpbSet);

  _agent.expect_serveJob_call ("job_9", make_list ("worker_9"));

  _scheduler.assignJobsToWorkers(); _scheduler.checkAllocations();
  workerList.clear();
  _scheduler.getListNotAllocatedWorkers(workerList);
  // check if there are any workers that are not yet reserved
  BOOST_CHECK(workerList.empty());

  // check if to worker_9 was assigned any job
  sdpa::job_id_t jobId = _scheduler.getAssignedJob(workerId);
  BOOST_CHECK(!jobId.str().empty());
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerGainsCpbLater)
{
  LOG(INFO, "Test the load-balancing when a worker gains a capability later ...");

  // number of workers
  const int nWorkers = 10;
  const int nJobs = 15;

  // create a give number of workers with different capabilities:
  std::ostringstream osstr;
  std::vector<sdpa::worker_id_t> arrWorkerIds;
  for(int k=0;k<nWorkers;k++)
  {
    osstr<<"worker_"<<k;
    sdpa::worker_id_t workerId(osstr.str());
    osstr.str("");
    arrWorkerIds.push_back(workerId);

    if( k<nWorkers-1 )
    {
        std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", workerId));
        sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
        _scheduler.addWorker(workerId, 1, cpbSet);
    }
    else
      _scheduler.addWorker(workerId, 1);
  }

  // submit a bunch of jobs now
  std::vector<sdpa::job_id_t> arrJobIds;
  for(int i=0;i<nJobs;i++)
  {
    osstr<<"job_"<<i;
    sdpa::job_id_t jobId(osstr.str());
    arrJobIds.push_back(jobId);
    osstr.str("");
    job_requirements_t job_reqs(requirement_list_t(1, we::type::requirement_t("C", true)), we::type::schedule_data(1, 100));
    _agent.addJob(jobId, "", sdpa::job_id_t(), false, "", job_reqs);
  }

  _agent.expect_serveJob_call ("job_0", make_list ("worker_8"));
  _agent.expect_serveJob_call ("job_1", make_list ("worker_7"));
  _agent.expect_serveJob_call ("job_2", make_list ("worker_5"));
  _agent.expect_serveJob_call ("job_3", make_list ("worker_4"));
  _agent.expect_serveJob_call ("job_4", make_list ("worker_6"));
  _agent.expect_serveJob_call ("job_5", make_list ("worker_3"));
  _agent.expect_serveJob_call ("job_6", make_list ("worker_2"));
  _agent.expect_serveJob_call ("job_7", make_list ("worker_1"));
  _agent.expect_serveJob_call ("job_8", make_list ("worker_0"));

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
  {
    _scheduler.schedule(jobId);
  }

  _scheduler.assignJobsToWorkers(); _scheduler.checkAllocations();

  // all the workers should have assigned jobs
  sdpa::worker_id_list_t workerList;
  _scheduler.getListNotAllocatedWorkers(workerList);
  // all workers should be assigned a job, excepting the last one,
  // which doesn't fit with the job reqs
  BOOST_CHECK_EQUAL (workerList.size(), 1);

  // the last worker gains now the missing capability
  //and will eventually receive one job ...

  osstr.str("");
  osstr<<"worker_"<<nWorkers-1;
  sdpa::worker_id_t lastWorkerId(osstr.str());
  std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", lastWorkerId));
  sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
  _scheduler.addCapabilities(lastWorkerId, cpbSet);

  _agent.expect_serveJob_call ("job_9", make_list ("worker_9"));

  // assign jobs to workers
  _scheduler.assignJobsToWorkers(); _scheduler.checkAllocations();
  workerList.clear();
  _scheduler.getListNotAllocatedWorkers(workerList);
  // all workers should be assigned a job, including the last one
  BOOST_CHECK(workerList.empty());
}

BOOST_AUTO_TEST_CASE(testCoallocSched)
{
  LOG(INFO, "Test the co-allocation ...");

  const int NWORKERS = 12;
  const std::string WORKER_CPBS[] = {"A", "B", "C"};

  std::ostringstream oss;

  // add a couple of workers
  for( int k=0; k<NWORKERS; k++ )
  {
    oss.str("");
    oss<<k;

    sdpa::worker_id_t workerId(oss.str());
    std::string cpbName(WORKER_CPBS[k%3]);
    sdpa::capability_t cpb(cpbName, "virtual", workerId);
    sdpa::capabilities_set_t cpbSet;
    cpbSet.insert(cpb);
    _scheduler.addWorker(workerId, 1, cpbSet);
  }

  // create a number of jobs
  const sdpa::job_id_t jobId0("Job0");
  job_requirements_t jobReqs0(requirement_list_t(1, we::type::requirement_t(WORKER_CPBS[0], true)), we::type::schedule_data(4, 100));
  _agent.addJob(jobId0, "description 0", sdpa::job_id_t(), false, "", jobReqs0);

  const sdpa::job_id_t jobId1("Job1");
  job_requirements_t jobReqs1(requirement_list_t(1, we::type::requirement_t(WORKER_CPBS[1], true)), we::type::schedule_data(4, 100));
  _agent.addJob(jobId1, "description 1", sdpa::job_id_t(), false, "", jobReqs1);

  const sdpa::job_id_t jobId2("Job2");
  job_requirements_t jobReqs2(requirement_list_t(1, we::type::requirement_t(WORKER_CPBS[2], true)), we::type::schedule_data(4, 100));
  _agent.addJob(jobId2, "description 2", sdpa::job_id_t(), false, "", jobReqs2);

  _agent.expect_serveJob_call (jobId0, make_list ("6", "3", "9", "0"));
  _agent.expect_serveJob_call (jobId1, make_list ("10", "7", "4", "1"));
  _agent.expect_serveJob_call (jobId2, make_list ("8", "11", "2", "5"));
  _scheduler.schedule(jobId0);
  _scheduler.schedule(jobId1);
  _scheduler.schedule(jobId2);

  _scheduler.assignJobsToWorkers();

  std::ostringstream ossrw;int k=-1;
  sdpa::worker_id_list_t listJobAssignedWorkers = _scheduler.getListAllocatedWorkers(jobId0);
  BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
  {
    k = boost::lexical_cast<int>(wid);
    BOOST_CHECK( k==0 || k==3 || k==6 || k==9);
  }
  LOG(INFO, "The job jobId0 has been allocated the workers "<<listJobAssignedWorkers);

  listJobAssignedWorkers.clear();
  listJobAssignedWorkers = _scheduler.getListAllocatedWorkers(jobId1);
  BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
  {
    k = boost::lexical_cast<int>(wid);
    BOOST_CHECK( k==1 || k==4 || k==7 || k==10);
  }
  LOG(INFO, "The job jobId1 has been allocated the workers "<<listJobAssignedWorkers);

  listJobAssignedWorkers.clear();
  listJobAssignedWorkers = _scheduler.getListAllocatedWorkers(jobId2);
  BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
  {
    k = boost::lexical_cast<int>(wid);
    BOOST_CHECK( k==2 || k==5 || k==8 || k==11);
  }
  LOG(INFO, "The job jobId2 has been allocated the workers "<<listJobAssignedWorkers);

  // try now to schedule a job requiring 2 resources of type "A"
  const sdpa::job_id_t jobId4("Job4");
  job_requirements_t jobReqs4(requirement_list_t(1, we::type::requirement_t(WORKER_CPBS[0], true)), we::type::schedule_data(2, 100));
  _agent.addJob(jobId4, "description 4", sdpa::job_id_t(), false, "", jobReqs4);

  _agent.expect_serveJob_call (jobId4, make_list ("6", "3"));
  _scheduler.schedule(jobId4);

  _scheduler.assignJobsToWorkers();
  sdpa::worker_id_list_t listFreeWorkers(_scheduler.getListAllocatedWorkers(jobId4));
  BOOST_CHECK(listFreeWorkers.empty());

  // Now report that jobId0 has finished and try to assign again resources to the job 4
  _scheduler.releaseReservation(jobId0);

  //listFreeWorkers.clear();
  _scheduler.assignJobsToWorkers();

  listFreeWorkers = _scheduler.getListAllocatedWorkers(jobId4);
  BOOST_CHECK(!listFreeWorkers.empty());

  int w0 = boost::lexical_cast<int>(listFreeWorkers.front());
  BOOST_CHECK(w0==0 || w0 == 3  || w0 == 6|| w0 == 9);

  int w1 = boost::lexical_cast<int>(*(boost::next(listFreeWorkers.begin())));
  BOOST_CHECK(w1==0 || w1 == 3  || w1 == 6|| w1 == 9);
}

BOOST_AUTO_TEST_CASE(tesLBStopRestartWorker)
{
  LOG(INFO, "Test the load-balancing when a worker is stopped, re-started and announces afterwards its capabilities ...");

  // number of workers
  const int nWorkers = 10;
  const int nJobs = 10;

  // create a give number of workers with different capabilities:
  std::ostringstream osstr;
  std::vector<sdpa::worker_id_t> arrWorkerIds;
  for(int k=0;k<nWorkers;k++)
  {
    osstr<<"worker_"<<k;
    sdpa::worker_id_t workerId(osstr.str());
    osstr.str("");
    arrWorkerIds.push_back(workerId);
    std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", workerId));
    sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
    _scheduler.addWorker(workerId, 1, cpbSet);
  }

  // submit a bunch of jobs now
  std::vector<sdpa::job_id_t> arrJobIds;
  for(int i=0;i<nJobs;i++)
  {
    osstr<<"job_"<<i;
    sdpa::job_id_t jobId(osstr.str());
    arrJobIds.push_back(jobId);
    osstr.str("");
    _agent.addJob(jobId, "", sdpa::job_id_t(), false, "",  job_requirements_t(requirement_list_t(1, we::type::requirement_t("C", true)), we::type::schedule_data(1, 100)));
  }

  _agent.expect_serveJob_call ("job_0", make_list ("worker_9"));
  _agent.expect_serveJob_call ("job_1", make_list ("worker_8"));
  _agent.expect_serveJob_call ("job_2", make_list ("worker_7"));
  _agent.expect_serveJob_call ("job_3", make_list ("worker_5"));
  _agent.expect_serveJob_call ("job_4", make_list ("worker_4"));
  _agent.expect_serveJob_call ("job_5", make_list ("worker_6"));
  _agent.expect_serveJob_call ("job_6", make_list ("worker_3"));
  _agent.expect_serveJob_call ("job_7", make_list ("worker_2"));
  _agent.expect_serveJob_call ("job_8", make_list ("worker_1"));
  _agent.expect_serveJob_call ("job_9", make_list ("worker_0"));

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
  {
    _scheduler.schedule(jobId);
  }

  _scheduler.assignJobsToWorkers();
  _scheduler.checkAllocations();

  LOG(DEBUG, "Initial allocations ...");
  // all the workers should have assigned jobs
  sdpa::worker_id_list_t workerList;
  _scheduler.getListNotAllocatedWorkers(workerList);
  // check if there are any workers that are not yet reserved
  BOOST_CHECK(workerList.empty());

  sdpa::worker_id_t lastWorkerId("worker_9");
  sdpa::job_id_t jobId = _scheduler.getAssignedJob(lastWorkerId);
  LOG(DEBUG, "The worker "<<lastWorkerId<<" was assigned the job "<<jobId);
  sdpa::job_id_t oldJobId(jobId);

  LOG(DEBUG, "The worker "<<lastWorkerId<<" has the job "<<jobId<<" assigned");

  // and now simply delete the last worker !
  LOG(DEBUG, "Reschedule the jobs assigned to "<<lastWorkerId<<"!");
  _scheduler.rescheduleWorkerJob(lastWorkerId, jobId);

  _agent.expect_serveJob_call ("job_0", make_list ("worker_9"));
  _scheduler.schedule(jobId);
  BOOST_CHECK (_scheduler.schedulingAllowed());

  LOG(DEBUG, "Delete the worker "<<lastWorkerId<<"!");
  _scheduler.deleteWorker(lastWorkerId);
  sdpa::worker_id_list_t listW = _scheduler.getListAllocatedWorkers(jobId);
  BOOST_CHECK(listW.empty());
  LOG_IF(DEBUG, listW.empty(), "The worker "<<lastWorkerId<<" was deleted!");

  std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", lastWorkerId));
  sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
  _scheduler.addWorker(lastWorkerId, 1, cpbSet);

  LOG(DEBUG, "The worker "<<lastWorkerId<<" was re-added!");
  _scheduler.assignJobsToWorkers();
  _scheduler.checkAllocations();

  BOOST_REQUIRE_EQUAL (oldJobId, _scheduler.getAssignedJob (lastWorkerId));
}

BOOST_AUTO_TEST_SUITE_END()
