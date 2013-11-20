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
#include <sdpa/daemon/agent/Agent.hpp>
#include <sdpa/daemon/scheduler/SimpleScheduler.hpp>
#include "kvs_setup_fixture.hpp"

using namespace std;
using namespace sdpa::daemon;

const int NWORKERS = 12;
const int NJOBS    = 4;

const std::string WORKER_CPBS[] = {"A", "B", "C"};

typedef std::map<sdpa::job_id_t, sdpa::worker_id_t> mapJob2Worker_t;

// deriveaza o clasa TestAgent si override the methods SendEventOSlaves that's all
class TestAgent : public sdpa::daemon::Agent
{
public:
  typedef sdpa::shared_ptr<TestAgent > ptr_t;
  TestAgent( const std::string& name
             , const std::string& url
             , const sdpa::master_info_list_t& arrMasterNames
             , const unsigned int rank = 0
             , const boost::optional<std::string>& appGuiUrl = boost::none)
    : sdpa::daemon::Agent(name, url, arrMasterNames, rank, appGuiUrl)
  {
  }

  void serveJob(const sdpa::worker_id_t& wid, const sdpa::job_id_t& jobId)
  {
      DLOG(TRACE, "Submit the job "<<jobId<<" to the worker "<<wid
                                  <<". This message can be ignored.");
  }


  void serveJob(const sdpa::worker_id_list_t& worker_list, const sdpa::job_id_t& jobId)
  {
    DLOG(TRACE, "Submit the job "<<jobId<<" to each of these workers: "<<worker_list
                                <<". This message can be ignored.");
  }

  void submitWorkflow(const id_type& id, const encoded_type& )
  {
    DLOG(TRACE, "The agent is trying to forward the master job "<<id<<" to the workflow engine");
    BOOST_REQUIRE(hasWorkflowEngine());
  }

  void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& pEvt)
  {
    DLOG(TRACE, "The agent is trying to send a message of type "<<pEvt->str()<<" to the daemon stage");
    BOOST_REQUIRE(daemon_stage().lock());
  }

  void sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& pEvt)
  {
    DLOG(TRACE, "The agent is trying to send a message of type "<<pEvt->str()<<" to the master stage");
    BOOST_REQUIRE(to_master_stage());
  }

  void sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& pEvt)
  {
    DLOG(TRACE, "The agent is trying to send a message of type "<<pEvt->str()<<" to the slave stage");
    BOOST_REQUIRE(to_slave_stage());
  }
};

struct MyFixture
{
    MyFixture()
    {
      try {
          m_pAgent = new TestAgent("agent", "127.0.0.1", sdpa::master_info_list_t());
      }
      catch(const std::bad_alloc&) {
          m_pAgent = NULL;
      }

      BOOST_REQUIRE(m_pAgent!=NULL);
    }

    ~MyFixture()
    {
      delete m_pAgent;
    }

    TestAgent* m_pAgent;
};

BOOST_FIXTURE_TEST_SUITE( test_Scheduler, MyFixture )

BOOST_GLOBAL_FIXTURE (KVSSetup)

/*
BOOST_AUTO_TEST_CASE(testCapabilitiesMatching)
{
  LOG( INFO, "Test if the capabilities are matching the requirements "<<std::endl);
  sdpa::daemon::SimpleScheduler::ptr_t ptrScheduler(new sdpa::daemon::SimpleScheduler(m_pAgent));

  LOG_IF(ERROR, !ptrScheduler, "The scheduler was not properly initialized");
  BOOST_REQUIRE(ptrScheduler);

  sdpa::worker_id_t workerId("test_worker");
  sdpa::capabilities_set_t workerCpbSet;

  workerCpbSet.insert(sdpa::capability_t("A","",workerId));
  workerCpbSet.insert(sdpa::capability_t("B","",workerId));
  workerCpbSet.insert(sdpa::capability_t("C","",workerId));

  ptrScheduler->addWorker(workerId, 1, workerCpbSet);

  // check what are the agent's capabilites now
  sdpa::capabilities_set_t acquiredCpbs;
  ptrScheduler->getWorkerCapabilities(workerId, acquiredCpbs);
  bool bSameCpbs(workerCpbSet==acquiredCpbs);
  BOOST_REQUIRE(bSameCpbs);
  LOG_IF(ERROR, !bSameCpbs, "The worker doesn't have the expected capabilities!");

  // Now, create a job that requires the capabilities A and B
  requirement_list_t reqList;
  reqList.push_back(requirement_t("A", true));
  reqList.push_back(requirement_t("B", true));
  job_requirements_t jobReqs(reqList, schedule_data());

  // check if there is any matching worker
  sdpa::worker_id_list_t wlist(1, workerId);
  sdpa::worker_id_t matchingWorkerId(ptrScheduler->findSuitableWorker(jobReqs, wlist));
  LOG_IF(ERROR, workerId!=matchingWorkerId, "The worker found is not the expected one!");
  BOOST_REQUIRE(workerId==matchingWorkerId);
}

BOOST_AUTO_TEST_CASE(testGainCap)
{
  LOG(INFO, "Test scheduling when the required capabilities are gained later ...");
  sdpa::daemon::SimpleScheduler::ptr_t ptrScheduler(new sdpa::daemon::SimpleScheduler(m_pAgent));

  LOG_IF(ERROR, !ptrScheduler, "The scheduler was not properly initialized");
  BOOST_REQUIRE(ptrScheduler);

  sdpa::worker_id_t worker_A("worker_A");

  sdpa::capabilities_set_t cpbSetA;
  ptrScheduler->addWorker(worker_A, 1, cpbSetA);

  const sdpa::job_id_t jobId1("Job1");
  sdpa::daemon::Job::ptr_t pJob1(new Job(jobId1, "description 1", sdpa::job_id_t()));
  job_requirements_t jobReqs1(requirement_list_t(1, requirement_t("C", true)), schedule_data(1, 100));
  m_pAgent->addJob(jobId1, pJob1, jobReqs1);

  LOG(DEBUG, "Schedule the job "<<jobId1);
  ptrScheduler->schedule_remotely(jobId1);

  ptrScheduler->assignJobsToWorkers();

  sdpa::worker_id_t assgnWid(ptrScheduler->getAssignedWorker(jobId1));
  BOOST_REQUIRE(assgnWid.empty());

  if(assgnWid == worker_A)
    LOG(DEBUG, "The job Job1 was scheduled on worker_A, which is incorrect, because worker_A doesn't have yet the capability \"C\"");
  else
    LOG(DEBUG, "The job Job1 wasn't scheduled on worker_A, which is correct, as it hasn't yet acquired the capability \"C\"");

  sdpa::capability_t cpb1("C", "virtual", worker_A);
  cpbSetA.insert(cpb1);
  ptrScheduler->addCapabilities(worker_A, cpbSetA);

  LOG(DEBUG, "Check if worker_A really acquired the capability \"C\"");

  sdpa::capabilities_set_t cpbset;
  ptrScheduler->getWorkerCapabilities(worker_A, cpbset);

  LOG(DEBUG, "The worker_A has now the following capabilities: ["<<cpbset<<"]");

  LOG(DEBUG, "Try to assign again jobs to the workers ...");
  ptrScheduler->assignJobsToWorkers();

  assgnWid = ptrScheduler->getAssignedWorker(jobId1);
  BOOST_REQUIRE(assgnWid == worker_A);
  if(assgnWid == worker_A)
    LOG(DEBUG, "The job Job1 was scheduled on worker_A, which is correct, as the worker_A has now gained the capability \"C\"");
  else
    LOG(DEBUG, "The job Job1 wasn't scheduled on worker_A, despite the fact is is the only one having the required  capability, which is incorrect");
}

BOOST_AUTO_TEST_CASE(testLoadBalancing)
{
  LOG(INFO, "testLoadBalancing");
  sdpa::daemon::SimpleScheduler::ptr_t ptrScheduler(new sdpa::daemon::SimpleScheduler(m_pAgent));

  LOG_IF(ERROR, !ptrScheduler, "The scheduler was not properly initialized");
  BOOST_REQUIRE(ptrScheduler);

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
      ptrScheduler->addWorker(workerId, 1, cpbSet);
  }

  // submit a bunch of jobs now
  std::list<sdpa::job_id_t> listJobIds;
  for(int i=0;i<nJobs;i++)
  {
      osstr<<"job_"<<i;
      sdpa::job_id_t jobId(osstr.str());
      listJobIds.push_back(jobId);
      osstr.str("");
      sdpa::daemon::Job::ptr_t pJob(new Job(jobId, "", sdpa::job_id_t()));
      m_pAgent->addJob(jobId, pJob, job_requirements_t(requirement_list_t(1, requirement_t("C", true)), schedule_data(1, 100)));
  }

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, listJobIds)
  {
    ptrScheduler->schedule_remotely(jobId);
  }

  ptrScheduler->assignJobsToWorkers();

  // check here if all workers have distinct jobs assigned
  sdpa::worker_id_list_t workerList;
  ptrScheduler->getWorkerList(workerList);

  while(!listJobIds.empty())
  {
    sdpa::job_id_t jobId = listJobIds.front();
    // check if the job was assigned to any worker
    sdpa::worker_id_t assgnWid(ptrScheduler->getAssignedWorker(jobId));
    BOOST_REQUIRE(!assgnWid.empty());
    workerList.remove(assgnWid);
    listJobIds.pop_front();
  }

  BOOST_REQUIRE(listJobIds.empty());
  BOOST_REQUIRE(workerList.empty());
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerJoinsLater)
{
  LOG(INFO, "Test the load-balancing when a worker joins later ...");
  sdpa::daemon::SimpleScheduler::ptr_t ptrScheduler(new sdpa::daemon::SimpleScheduler(m_pAgent));

  LOG_IF(ERROR, !ptrScheduler, "The scheduler was not properly initialized");
  BOOST_REQUIRE(ptrScheduler);

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
      ptrScheduler->addWorker(workerId, 1, cpbSet);
  }

  // submit a bunch of jobs now
  std::list<sdpa::job_id_t> listJobIds;
  for(int i=0;i<nJobs;i++)
  {
      osstr<<"job_"<<i;
      sdpa::job_id_t jobId(osstr.str());
      listJobIds.push_back(jobId);
      osstr.str("");
      sdpa::daemon::Job::ptr_t pJob(new Job(jobId, "", sdpa::job_id_t()));
      m_pAgent->addJob(jobId, pJob, job_requirements_t(requirement_list_t(1, requirement_t("C", true)), schedule_data(1, 100)));
  }

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, listJobIds)
  {
      ptrScheduler->schedule_remotely(jobId);
  }

  ptrScheduler->assignJobsToWorkers();

   // check here if all workers have distinct jobs assigned
   sdpa::worker_id_list_t workerList;
   ptrScheduler->getWorkerList(workerList);

   while(listJobIds.size()!=1)
   {
      sdpa::job_id_t jobId = listJobIds.front();
      // check if the job was assigned to any worker
      sdpa::worker_id_t assgnWid(ptrScheduler->getAssignedWorker(jobId));
      BOOST_REQUIRE(!assgnWid.empty());
      workerList.remove(assgnWid);
      listJobIds.pop_front();
   }

   // add new worker now (worker_9)...
   osstr<<"worker_"<<nWorkers-1;
   sdpa::worker_id_t workerId(osstr.str());
   osstr.str("");
   arrWorkerIds.push_back(workerId);
   std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", workerId));
   ptrScheduler->addWorker(workerId, 1, sdpa::capabilities_set_t(arrCpbs.begin(), arrCpbs.end()));

   ptrScheduler->assignJobsToWorkers();

   sdpa::job_id_t jobId = listJobIds.front();
   // check if the job was assigned to any worker
   sdpa::worker_id_t assgnWid = ptrScheduler->getAssignedWorker(jobId);
   BOOST_REQUIRE(!assgnWid.empty());
   workerList.remove(assgnWid);
   listJobIds.pop_front();

   // check if there are any jobs non-asssigned left
   BOOST_REQUIRE(listJobIds.empty());
   // check if all the workers were served
   BOOST_REQUIRE(workerList.empty());
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerGainsCpbLater)
{
  LOG(INFO, "Test the load-balancing when a worker gains a capability later ...");

  sdpa::daemon::SimpleScheduler::ptr_t ptrScheduler(new sdpa::daemon::SimpleScheduler(m_pAgent));

  LOG_IF(ERROR, !ptrScheduler, "The scheduler was not properly initialized");
  BOOST_REQUIRE(ptrScheduler);

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
        ptrScheduler->addWorker(workerId, 1, cpbSet);
    }
    else
      ptrScheduler->addWorker(workerId, 1); // the last worker has no capability, yet
  }

  // submit a bunch of jobs now
  std::list<sdpa::job_id_t> listJobIds;
  for(int i=0;i<nJobs;i++)
  {
    osstr<<"job_"<<i;
    sdpa::job_id_t jobId(osstr.str());
    listJobIds.push_back(jobId);
    osstr.str("");
    sdpa::daemon::Job::ptr_t pJob(new Job(jobId, "", sdpa::job_id_t()));
    m_pAgent->addJob(jobId, pJob, job_requirements_t(requirement_list_t(1, requirement_t("C", true)), schedule_data(1, 100)));
  }

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, listJobIds)
  {
    ptrScheduler->schedule_remotely(jobId);
  }

  ptrScheduler->assignJobsToWorkers();

  // check here if all workers have distinct jobs assigned
  sdpa::worker_id_list_t workerList;
  ptrScheduler->getWorkerList(workerList);

  while(listJobIds.size()!=1)
  {
      sdpa::job_id_t jobId = listJobIds.front();
      // check if the job was assigned to any worker
      sdpa::worker_id_t assgnWid(ptrScheduler->getAssignedWorker(jobId));
      if(!assgnWid.empty())
      {
        workerList.remove(assgnWid);
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
  ptrScheduler->addCapabilities(lastWorkerId, cpbSet);

  BOOST_REQUIRE(ptrScheduler->findWorker(lastWorkerId)->nbAllocatedJobs()==0);

  ptrScheduler->assignJobsToWorkers();

  sdpa::job_id_t jobId = listJobIds.front();
  // check if the job was assigned to any worker
  sdpa::worker_id_t assgnWid = ptrScheduler->getAssignedWorker(jobId);
  BOOST_REQUIRE(assgnWid == lastWorkerId);
  workerList.remove(assgnWid);
  listJobIds.pop_front();

  // check if there are any jobs non-asssigned left
  BOOST_REQUIRE(listJobIds.empty());
  // check if all the workers were served
  BOOST_REQUIRE(workerList.empty());
}
*/

BOOST_AUTO_TEST_CASE(tesLBStopRestartWorker)
{
  LOG(INFO, "Test the load-balancing when a worker is stopped, re-started and announces afterwards its capabilities ...");

  sdpa::daemon::SimpleScheduler::ptr_t ptrScheduler(new sdpa::daemon::SimpleScheduler(m_pAgent));

  LOG_IF(ERROR, !ptrScheduler, "The scheduler was not properly initialized");
  BOOST_REQUIRE(ptrScheduler);

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
    ptrScheduler->addWorker(workerId, 1, cpbSet);
  }

  // submit a bunch of jobs now
  std::vector<sdpa::job_id_t> listJobIds;
  for(int i=0;i<nJobs;i++)
  {
    osstr<<"job_"<<i;
    sdpa::job_id_t jobId(osstr.str());
    listJobIds.push_back(jobId);
    osstr.str("");
    sdpa::daemon::Job::ptr_t pJob(new Job(jobId, "", sdpa::job_id_t()));
    m_pAgent->addJob(jobId, pJob,  job_requirements_t(requirement_list_t(1, requirement_t("C", true)), schedule_data(1, 100)));
  }

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, listJobIds)
  {
    ptrScheduler->schedule_remotely(jobId);
  }

  ptrScheduler->assignJobsToWorkers();

  BOOST_FOREACH(const sdpa::worker_id_t& wid, arrWorkerIds)
  {
    BOOST_REQUIRE(ptrScheduler->findWorker(wid)->nbAllocatedJobs()==1);
  }

  osstr.str("");
  osstr<<"worker_"<<nWorkers-1;
  sdpa::worker_id_t lastWorkerId(osstr.str());

  LOG(DEBUG, "Delete the worker "<<lastWorkerId<<"!");
  ptrScheduler->deleteWorker(lastWorkerId);

  std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", lastWorkerId));
  sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
  ptrScheduler->addWorker(lastWorkerId, 1, cpbSet);

  BOOST_REQUIRE(ptrScheduler->findWorker(lastWorkerId)->nbAllocatedJobs()==0);

  LOG(DEBUG, "The worker "<<lastWorkerId<<" was re-added!");
  ptrScheduler->assignJobsToWorkers();

  BOOST_FOREACH(const sdpa::worker_id_t& wid, arrWorkerIds)
  {
    BOOST_REQUIRE(ptrScheduler->findWorker(wid)->nbAllocatedJobs()==1);
  }
}

BOOST_AUTO_TEST_SUITE_END()
