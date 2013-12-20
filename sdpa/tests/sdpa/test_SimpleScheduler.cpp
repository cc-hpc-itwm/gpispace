/*
 * =====================================================================================
 *
 *       Filename:  test_Scheduler.cpp
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
#define BOOST_TEST_MODULE TestSimpleScheduler
#include <boost/test/unit_test.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/scheduler/SimpleScheduler.hpp>
#include "kvs_setup_fixture.hpp"

#include <fhg/util/boost/test/printer/list.hpp>
#include <fhg/util/boost/test/printer/set.hpp>

BOOST_TEST_DONT_PRINT_LOG_VALUE (sdpa::Capability)
BOOST_TEST_DONT_PRINT_LOG_VALUE (sdpa::job_id_t)

using namespace std;
using namespace sdpa::daemon;

const std::string WORKER_CPBS[] = {"A", "B", "C"};

typedef std::map<sdpa::job_id_t, sdpa::worker_id_t> mapJob2Worker_t;

class TestOrchestrator : public sdpa::daemon::Orchestrator
{
public:
  typedef boost::shared_ptr<TestOrchestrator > ptr_t;
  TestOrchestrator( const std::string& name, const std::string& url)
    : sdpa::daemon::Orchestrator(name, url)
  {
  }

  void serveJob(const sdpa::worker_id_list_t& worker_list, const sdpa::job_id_t& jobId)
  {
    DLOG(TRACE, "Submit the job "<<jobId<<" to each of these workers: "<<worker_list
                                <<". This message can be ignored.");
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
};

struct allocate_test_orchestrator_and_scheduler
{
    allocate_test_orchestrator_and_scheduler()
      : _orchestrator ("orchestrator", "127.0.0.1")
      , _scheduler (&_orchestrator)
    {}

    TestOrchestrator _orchestrator;
    sdpa::daemon::SimpleScheduler _scheduler;
};

BOOST_FIXTURE_TEST_SUITE( test_Scheduler, allocate_test_orchestrator_and_scheduler)

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
  _orchestrator.addJob(jobId1, "description 1", sdpa::job_id_t(), false, "", jobReqs1);

  LOG(DEBUG, "Schedule the job "<<jobId1);
  _scheduler.schedule(jobId1);

  _scheduler.assignJobsToWorkers();

  BOOST_REQUIRE_EQUAL (_scheduler.getAssignedWorker (jobId1), boost::none);

  sdpa::capability_t cpb1("C", "virtual", worker_A);
  cpbSetA.insert(cpb1);
  _scheduler.addCapabilities(worker_A, cpbSetA);

  LOG(DEBUG, "Check if worker_A really acquired the capability \"C\"");

  sdpa::capabilities_set_t cpbset;
  _scheduler.getWorkerCapabilities(worker_A, cpbset);

  LOG(DEBUG, "The worker_A has now the following capabilities: ["<<cpbset<<"]");

  LOG(DEBUG, "Try to assign again jobs to the workers ...");
  _scheduler.assignJobsToWorkers();

  BOOST_REQUIRE_EQUAL (_scheduler.getAssignedWorker (jobId1), worker_A);
}


BOOST_AUTO_TEST_CASE(testLoadBalancing)
{
  LOG(INFO, "testLoadBalancing");

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
  std::list<sdpa::job_id_t> listJobIds;
  for(int i=0;i<nJobs;i++)
  {
      osstr<<"job_"<<i;
      sdpa::job_id_t jobId(osstr.str());
      listJobIds.push_back(jobId);
      osstr.str("");
      _orchestrator.addJob(jobId, "", sdpa::job_id_t(), false, "", job_requirements_t(requirement_list_t(1, we::type::requirement_t("C", true)), we::type::schedule_data(1, 100)));
  }

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, listJobIds)
  {
    _scheduler.schedule(jobId);
  }

  _scheduler.assignJobsToWorkers();

  // check here if all workers have distinct jobs assigned
  sdpa::worker_id_list_t workerList;
  _scheduler.getWorkerList(workerList);

  while(!listJobIds.empty())
  {
    sdpa::job_id_t jobId = listJobIds.front();
    // check if the job was assigned to any worker
    boost::optional<sdpa::worker_id_t> assgnWid(_scheduler.getAssignedWorker(jobId));
    BOOST_REQUIRE(assgnWid);
    workerList.remove(*assgnWid);
    listJobIds.pop_front();
  }

  BOOST_REQUIRE(listJobIds.empty());
  BOOST_REQUIRE(workerList.empty());
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerJoinsLater)
{
  LOG(INFO, "Test the load-balancing when a worker joins later ...");

  // number of workers
  const int nWorkers = 10;
  const int nJobs = 10;

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
  std::list<sdpa::job_id_t> listJobIds;
  for(int i=0;i<nJobs;i++)
  {
      osstr<<"job_"<<i;
      sdpa::job_id_t jobId(osstr.str());
      listJobIds.push_back(jobId);
      osstr.str("");
      job_requirements_t job_reqs(requirement_list_t(1, we::type::requirement_t("C", true)), we::type::schedule_data(1, 100));
      _orchestrator.addJob(jobId, "", sdpa::job_id_t(), false, "", job_reqs);
  }

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, listJobIds)
  {
      _scheduler.schedule(jobId);
  }

  _scheduler.assignJobsToWorkers();

   // check here if all workers have distinct jobs assigned
   sdpa::worker_id_list_t workerList;
   _scheduler.getWorkerList(workerList);

   while(listJobIds.size()!=1)
   {
      sdpa::job_id_t jobId = listJobIds.front();
      // check if the job was assigned to any worker
      boost::optional<sdpa::worker_id_t> assgnWid(_scheduler.getAssignedWorker(jobId));
      BOOST_REQUIRE(assgnWid);
      workerList.remove(*assgnWid);
      listJobIds.pop_front();
   }

   // add new worker now (worker_9)...
   osstr<<"worker_"<<nWorkers-1;
   sdpa::worker_id_t workerId(osstr.str());
   osstr.str("");
   arrWorkerIds.push_back(workerId);
   std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", workerId));
   _scheduler.addWorker(workerId, 1, sdpa::capabilities_set_t(arrCpbs.begin(), arrCpbs.end()));

   _scheduler.assignJobsToWorkers();

   sdpa::job_id_t jobId = listJobIds.front();
   // check if the job was assigned to any worker
   boost::optional<sdpa::worker_id_t> assgnWid = _scheduler.getAssignedWorker(jobId);
   BOOST_REQUIRE(assgnWid);
   workerList.remove(*assgnWid);
   listJobIds.pop_front();

   // check if there are any jobs non-asssigned left
   BOOST_REQUIRE(listJobIds.empty());
   // check if all the workers were served
   BOOST_REQUIRE(workerList.empty());
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerGainsCpbLater)
{
  LOG(INFO, "Test the load-balancing when a worker gains a capability later ...");

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

    if( k<nWorkers-1 )
    {
        std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", workerId));
        sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
        _scheduler.addWorker(workerId, 1, cpbSet);
    }
    else
      _scheduler.addWorker(workerId, 1); // the last worker has no capability, yet
  }

  // submit a bunch of jobs now
  std::list<sdpa::job_id_t> listJobIds;
  for(int i=0;i<nJobs;i++)
  {
    osstr<<"job_"<<i;
    sdpa::job_id_t jobId(osstr.str());
    listJobIds.push_back(jobId);
    osstr.str("");
    job_requirements_t job_reqs(requirement_list_t(1, we::type::requirement_t("C", true)), we::type::schedule_data(1, 100));
    _orchestrator.addJob(jobId, "", sdpa::job_id_t(), false, "", job_reqs);
  }

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, listJobIds)
  {
    _scheduler.schedule(jobId);
  }

  _scheduler.assignJobsToWorkers();

  // check here if all workers have distinct jobs assigned
  sdpa::worker_id_list_t workerList;
  _scheduler.getWorkerList(workerList);

  while(listJobIds.size()!=1)
  {
      sdpa::job_id_t jobId = listJobIds.front();
      // check if the job was assigned to any worker
      boost::optional<sdpa::worker_id_t> assgnWid(_scheduler.getAssignedWorker(jobId));
      if(assgnWid)
      {
        workerList.remove(*assgnWid);
        listJobIds.pop_front();
      }
  }

  // the last worker gains now the missing capability
  //and will eventually receive one job ...

  osstr.str("");
  osstr<<"worker_"<<nWorkers-1;
  sdpa::worker_id_t lastWorkerId(osstr.str());
  std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", lastWorkerId));
  sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
  _scheduler.addCapabilities(lastWorkerId, cpbSet);

  BOOST_REQUIRE_EQUAL (_scheduler.findWorker(lastWorkerId)->nbAllocatedJobs(), 0);

  _scheduler.assignJobsToWorkers();

  sdpa::job_id_t jobId = listJobIds.front();
  // check if the job was assigned to any worker
  boost::optional<sdpa::worker_id_t> assgnWid = _scheduler.getAssignedWorker(jobId);
  BOOST_REQUIRE_EQUAL (*assgnWid, lastWorkerId);
  workerList.remove(*assgnWid);
  listJobIds.pop_front();

  // check if there are any jobs non-asssigned left
  BOOST_REQUIRE(listJobIds.empty());
  // check if all the workers were served
  BOOST_REQUIRE(workerList.empty());
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
  std::vector<sdpa::job_id_t> listJobIds;
  for(int i=0;i<nJobs;i++)
  {
    osstr<<"job_"<<i;
    sdpa::job_id_t jobId(osstr.str());
    listJobIds.push_back(jobId);
    osstr.str("");
    job_requirements_t job_reqs(requirement_list_t(1, we::type::requirement_t("C", true)), we::type::schedule_data(1, 100));
    _orchestrator.addJob(jobId, "", sdpa::job_id_t(), false, "", job_reqs);
  }

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, listJobIds)
  {
    _scheduler.schedule(jobId);
  }

  _scheduler.assignJobsToWorkers();

  BOOST_FOREACH(const sdpa::worker_id_t& wid, arrWorkerIds)
  {
    BOOST_REQUIRE_EQUAL (_scheduler.findWorker(wid)->nbAllocatedJobs(), 1);
  }

  osstr.str("");
  osstr<<"worker_"<<nWorkers-1;
  sdpa::worker_id_t lastWorkerId(osstr.str());

  LOG(DEBUG, "Delete the worker "<<lastWorkerId<<"!");
  _scheduler.deleteWorker(lastWorkerId);

  std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", lastWorkerId));
  sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
  _scheduler.addWorker(lastWorkerId, 1, cpbSet);

  BOOST_REQUIRE_EQUAL (_scheduler.findWorker(lastWorkerId)->nbAllocatedJobs(), 0);

  LOG(DEBUG, "The worker "<<lastWorkerId<<" was re-added!");
  _scheduler.assignJobsToWorkers();

  BOOST_FOREACH(const sdpa::worker_id_t& wid, arrWorkerIds)
  {
    BOOST_REQUIRE_EQUAL (_scheduler.findWorker(wid)->nbAllocatedJobs(), 1);
  }
}

BOOST_AUTO_TEST_SUITE_END()
