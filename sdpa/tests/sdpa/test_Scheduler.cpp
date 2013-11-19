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
#define BOOST_TEST_MODULE TestScheduler
#include <sdpa/daemon/Job.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>
#include <list>
#include <sdpa/memory.hpp>
#include <time.h>
#include <sdpa/util/util.hpp>
#include <fstream>

#include <seda/StageRegistry.hpp>

#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/agent/Agent.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/get_pointer.hpp>
#include <utils.hpp>

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
      DLOG(TRACE, "Submit the job "<<jobId<<" to thes worker "<<wid
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

BOOST_AUTO_TEST_CASE(testGainCap)
{
  LOG(INFO, "Test scheduling when the required capabilities are gained later ...");
  sdpa::daemon::CoallocationScheduler::ptr_t ptrScheduler(new sdpa::daemon::CoallocationScheduler(m_pAgent));

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

  ptrScheduler->assignJobsToWorkers(); ptrScheduler->checkAllocations();

  sdpa::worker_id_list_t listAssgnWks = ptrScheduler->getListAllocatedWorkers(jobId1);
  BOOST_CHECK(listAssgnWks.empty());

  if(listAssgnWks == sdpa::worker_id_list_t(1,worker_A))
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
  ptrScheduler->checkAllocations();

  listAssgnWks = ptrScheduler->getListAllocatedWorkers(jobId1);
  BOOST_CHECK(!listAssgnWks.empty());

  bool bOutcome = (listAssgnWks == sdpa::worker_id_list_t(1,worker_A));
  BOOST_CHECK(bOutcome);
  if(listAssgnWks == sdpa::worker_id_list_t(1,worker_A))
    LOG(DEBUG, "The job Job1 was scheduled on worker_A, which is correct, as the worker_A has now gained the capability \"C\"");
  else
    LOG(DEBUG, "The job Job1 wasn't scheduled on worker_A, despite the fact is is the only one having the required  capability, which is incorrect");
}

BOOST_AUTO_TEST_CASE(testLoadBalancing)
{
  LOG(INFO, "testLoadBalancing");
  sdpa::daemon::CoallocationScheduler::ptr_t ptrScheduler(new sdpa::daemon::CoallocationScheduler(m_pAgent));

  LOG_IF(ERROR, !ptrScheduler, "The scheduler was not properly initialized");
  BOOST_REQUIRE(ptrScheduler);

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
      ptrScheduler->addWorker(workerId, 1, cpbSet);
  }

  // submit a bunch of jobs now
  std::vector<sdpa::job_id_t> arrJobIds;
  for(int i=0;i<nJobs;i++)
  {
      osstr<<"job_"<<i;
      sdpa::job_id_t jobId(osstr.str());
      arrJobIds.push_back(jobId);
      osstr.str("");
      sdpa::daemon::Job::ptr_t pJob(new Job(jobId, "", sdpa::job_id_t()));
      //m_pAgent->addJob(jobId, pJob, requirement_list_t(1, requirement_t("C", true)), schedule_data(1, 100));
      m_pAgent->addJob(jobId, pJob, job_requirements_t(requirement_list_t(1, requirement_t("C", true)), schedule_data(1, 100)));
  }

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
  {
      ptrScheduler->schedule_remotely(jobId);
  }

  ptrScheduler->assignJobsToWorkers(); ptrScheduler->checkAllocations();

  sdpa::worker_id_list_t workerList;
  ptrScheduler->getListNotAllocatedWorkers(workerList);

  // check if there are any workers that are not yet reserved
  BOOST_CHECK(workerList.empty());

  BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
  {
      sdpa::worker_id_list_t listJobAssignedWorkers = ptrScheduler->getListAllocatedWorkers(jobId);

      BOOST_CHECK(listJobAssignedWorkers.size() <= 1);
      if(!listJobAssignedWorkers.empty())
      {
          // delete the job
          ptrScheduler->deleteWorkerJob(listJobAssignedWorkers.front(), jobId);
      }

      ptrScheduler->releaseReservation(jobId);
  }

  ptrScheduler->assignJobsToWorkers();
  ptrScheduler->checkAllocations();

  workerList.clear();
  ptrScheduler->getListNotAllocatedWorkers(workerList);

  // check if the list of reserved workers is NJOBS - NWORKERS
  BOOST_CHECK_EQUAL(workerList.size(), 5);
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerJoinsLater)
{
  LOG(INFO, "Test the load-balancing when a worker joins later ...");
  sdpa::daemon::CoallocationScheduler::ptr_t ptrScheduler(new sdpa::daemon::CoallocationScheduler(m_pAgent));

  LOG_IF(ERROR, !ptrScheduler, "The scheduler was not properly initialized");
  BOOST_REQUIRE(ptrScheduler);

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
      ptrScheduler->addWorker(workerId, 1, cpbSet);
  }

  // submit a bunch of jobs now
  std::vector<sdpa::job_id_t> arrJobIds;
  for(int i=0;i<nJobs;i++)
  {
      osstr<<"job_"<<i;
      sdpa::job_id_t jobId(osstr.str());
      arrJobIds.push_back(jobId);
      osstr.str("");
      sdpa::daemon::Job::ptr_t pJob(new Job(jobId, "", sdpa::job_id_t()));
      m_pAgent->addJob(jobId, pJob, job_requirements_t(requirement_list_t(1, requirement_t("C", true)), schedule_data(1, 100)));
  }

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
  {
      ptrScheduler->schedule_remotely(jobId);
  }

  ptrScheduler->assignJobsToWorkers(); ptrScheduler->checkAllocations();

  // all the workers should have assigned jobs
  sdpa::worker_id_list_t workerList;
  ptrScheduler->getListNotAllocatedWorkers(workerList);
  // check if there are any workers that are not yet reserved
  BOOST_CHECK(workerList.empty());

  // add new worker now (worker_9)...
  osstr<<"worker_"<<nWorkers-1;
  sdpa::worker_id_t workerId(osstr.str());
  osstr.str("");
  arrWorkerIds.push_back(workerId);
  std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", workerId));
  sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
  ptrScheduler->addWorker(workerId, 1, cpbSet);

  ptrScheduler->assignJobsToWorkers(); ptrScheduler->checkAllocations();
  workerList.clear();
  ptrScheduler->getListNotAllocatedWorkers(workerList);
  // check if there are any workers that are not yet reserved
  BOOST_CHECK(workerList.empty());

  // check if to worker_9 was assigned any job
  sdpa::job_id_t jobId = ptrScheduler->getAssignedJob(workerId);
  BOOST_CHECK(!jobId.str().empty());
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerGainsCpbLater)
{
  LOG(INFO, "Test the load-balancing when a worker gains a capability later ...");

  sdpa::daemon::CoallocationScheduler::ptr_t ptrScheduler(new sdpa::daemon::CoallocationScheduler(m_pAgent));

  LOG_IF(ERROR, !ptrScheduler, "The scheduler was not properly initialized");
  BOOST_REQUIRE(ptrScheduler);

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
        ptrScheduler->addWorker(workerId, 1, cpbSet);
    }
    else
      ptrScheduler->addWorker(workerId, 1);
  }

  // submit a bunch of jobs now
  std::vector<sdpa::job_id_t> arrJobIds;
  for(int i=0;i<nJobs;i++)
  {
    osstr<<"job_"<<i;
    sdpa::job_id_t jobId(osstr.str());
    arrJobIds.push_back(jobId);
    osstr.str("");
    sdpa::daemon::Job::ptr_t pJob(new Job(jobId, "", sdpa::job_id_t()));
    m_pAgent->addJob(jobId, pJob, job_requirements_t(requirement_list_t(1, requirement_t("C", true)), schedule_data(1, 100)));
  }

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
  {
    ptrScheduler->schedule_remotely(jobId);
  }

  ptrScheduler->assignJobsToWorkers(); ptrScheduler->checkAllocations();

  // all the workers should have assigned jobs
  sdpa::worker_id_list_t workerList;
  ptrScheduler->getListNotAllocatedWorkers(workerList);
  // all workers should be assigned a job, excepting the last one,
  // which doesn't fit with the job reqs
  BOOST_CHECK(workerList.size()==1);

  // the last worker gains now the missing capability
  //and will eventually receive one job ...

  osstr.str("");
  osstr<<"worker_"<<nWorkers-1;
  sdpa::worker_id_t lastWorkerId(osstr.str());
  std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", lastWorkerId));
  sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
  ptrScheduler->addCapabilities(lastWorkerId, cpbSet);

  //ptrScheduler->printAllocationTable();
  // assign jobs to workers
  ptrScheduler->assignJobsToWorkers(); ptrScheduler->checkAllocations();
  workerList.clear();
  ptrScheduler->getListNotAllocatedWorkers(workerList);
  // all workers should be assigned a job, including the last one
  BOOST_CHECK(workerList.empty());
  //ptrScheduler->printAllocationTable();
}

BOOST_AUTO_TEST_CASE(testCoallocSched)
{
  LOG(INFO, "Test the co-allocation ...");

  const int NWORKERS = 12;
  const std::string WORKER_CPBS[] = {"A", "B", "C"};

  std::string addrAg = "127.0.0.1";
  std::string strBackupOrch;
  std::ostringstream oss;

  sdpa::daemon::CoallocationScheduler::ptr_t ptrScheduler(new sdpa::daemon::CoallocationScheduler(m_pAgent));

   LOG_IF(ERROR, !ptrScheduler, "The scheduler was not properly initialized");
   BOOST_REQUIRE(ptrScheduler);

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
    ptrScheduler->addWorker(workerId, 1, cpbSet);
  }

  // create a number of jobs
  const sdpa::job_id_t jobId0("Job0");
  sdpa::daemon::Job::ptr_t pJob0(new sdpa::daemon::Job(jobId0, "description 0", sdpa::job_id_t()));
  job_requirements_t jobReqs0(requirement_list_t(1, requirement_t(WORKER_CPBS[0], true)), schedule_data(4, 100));
  m_pAgent->addJob(jobId0, pJob0, jobReqs0);

  const sdpa::job_id_t jobId1("Job1");
  sdpa::daemon::Job::ptr_t pJob1(new sdpa::daemon::Job(jobId1, "description 1", sdpa::job_id_t()));
  job_requirements_t jobReqs1(requirement_list_t(1, requirement_t(WORKER_CPBS[1], true)), schedule_data(4, 100));
  m_pAgent->addJob(jobId1, pJob1, jobReqs1);

  const sdpa::job_id_t jobId2("Job2");
  sdpa::daemon::Job::ptr_t pJob2(new sdpa::daemon::Job(jobId2, "description 2", sdpa::job_id_t()));
  job_requirements_t jobReqs2(requirement_list_t(1, requirement_t(WORKER_CPBS[2], true)), schedule_data(4, 100));
  m_pAgent->addJob(jobId2, pJob2, jobReqs2);

  ptrScheduler->schedule_remotely(jobId0);
  ptrScheduler->schedule_remotely(jobId1);
  ptrScheduler->schedule_remotely(jobId2);

  ptrScheduler->assignJobsToWorkers();

  std::ostringstream ossrw;int k=-1;
  sdpa::worker_id_list_t listJobAssignedWorkers = ptrScheduler->getListAllocatedWorkers(jobId0);
  BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
  {
    k = boost::lexical_cast<int>(wid);
    BOOST_CHECK( k==0 || k==3 || k==6 || k==9);
  }
  LOG(INFO, "The job jobId0 has been allocated the workers "<<listJobAssignedWorkers);

  listJobAssignedWorkers.clear();
  listJobAssignedWorkers = ptrScheduler->getListAllocatedWorkers(jobId1);
  BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
  {
    k = boost::lexical_cast<int>(wid);
    BOOST_CHECK( k==1 || k==4 || k==7 || k==10);
  }
  LOG(INFO, "The job jobId1 has been allocated the workers "<<listJobAssignedWorkers);

  listJobAssignedWorkers.clear();
  listJobAssignedWorkers = ptrScheduler->getListAllocatedWorkers(jobId2);
  BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
  {
    k = boost::lexical_cast<int>(wid);
    BOOST_CHECK( k==2 || k==5 || k==8 || k==11);
  }
  LOG(INFO, "The job jobId2 has been allocated the workers "<<listJobAssignedWorkers);

  // try now to schedule a job requiring 2 resources of type "A"
  const sdpa::job_id_t jobId4("Job4");
  sdpa::daemon::Job::ptr_t pJob4(new sdpa::daemon::Job(jobId4, "description 4", sdpa::job_id_t()));
  job_requirements_t jobReqs4(requirement_list_t(1, requirement_t(WORKER_CPBS[0], true)), schedule_data(2, 100));
  m_pAgent->addJob(jobId4, pJob4, jobReqs4);

  ptrScheduler->schedule_remotely(jobId4);

  ptrScheduler->assignJobsToWorkers();
  sdpa::worker_id_list_t listFreeWorkers(ptrScheduler->getListAllocatedWorkers(jobId4));
  BOOST_CHECK(listFreeWorkers.empty());

  // Now report that jobId0 has finished and try to assign again resources to the job 4
  ptrScheduler->releaseReservation(jobId0);

  //listFreeWorkers.clear();
  ptrScheduler->assignJobsToWorkers();

  listFreeWorkers = ptrScheduler->getListAllocatedWorkers(jobId4);
  BOOST_CHECK(!listFreeWorkers.empty());

  int w0 = boost::lexical_cast<int>(listFreeWorkers.front());
  BOOST_CHECK(w0==0 || w0 == 3  || w0 == 6|| w0 == 9);

  int w1 = boost::lexical_cast<int>(*(boost::next(listFreeWorkers.begin())));
  BOOST_CHECK(w1==0 || w1 == 3  || w1 == 6|| w1 == 9);
}

BOOST_AUTO_TEST_CASE(tesLBStopRestartWorker)
{
  LOG(INFO, "Test the load-balancing when a worker is stopped, re-started and announces afterwards its capabilities ...");

  sdpa::daemon::CoallocationScheduler::ptr_t ptrScheduler(new sdpa::daemon::CoallocationScheduler(m_pAgent));

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
  std::vector<sdpa::job_id_t> arrJobIds;
  for(int i=0;i<nJobs;i++)
  {
    osstr<<"job_"<<i;
    sdpa::job_id_t jobId(osstr.str());
    arrJobIds.push_back(jobId);
    osstr.str("");
    sdpa::daemon::Job::ptr_t pJob(new Job(jobId, "", sdpa::job_id_t()));
    m_pAgent->addJob(jobId, pJob,  job_requirements_t(requirement_list_t(1, requirement_t("C", true)), schedule_data(1, 100)));
  }

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
  {
    ptrScheduler->schedule_remotely(jobId);
  }

  ptrScheduler->assignJobsToWorkers();
  ptrScheduler->checkAllocations();

  LOG(DEBUG, "Initial allocations ...");
  ptrScheduler->printAllocationTable();
  // all the workers should have assigned jobs
  sdpa::worker_id_list_t workerList;
  ptrScheduler->getListNotAllocatedWorkers(workerList);
  // check if there are any workers that are not yet reserved
  BOOST_CHECK(workerList.empty());

  sdpa::worker_id_t lastWorkerId("worker_9");
  sdpa::job_id_t jobId = ptrScheduler->getAssignedJob(lastWorkerId);
  LOG(DEBUG, "The worker "<<lastWorkerId<<" was assigned the job "<<jobId);
  sdpa::job_id_t oldJobId(jobId);

  LOG(DEBUG, "The worker "<<lastWorkerId<<" has the job "<<jobId<<" assigned");

  // and now simply delete the last worker !
  LOG(DEBUG, "Reschedule the jobs assigned to "<<lastWorkerId<<"!");
  ptrScheduler->rescheduleWorkerJob(lastWorkerId, jobId);

  ptrScheduler->schedule_remotely(jobId);
  BOOST_CHECK (ptrScheduler->schedulingAllowed());

  LOG(DEBUG, "Delete the worker "<<lastWorkerId<<"!");
  ptrScheduler->deleteWorker(lastWorkerId);
  sdpa::worker_id_list_t listW = ptrScheduler->getListAllocatedWorkers(jobId);
  BOOST_CHECK(listW.empty());
  LOG_IF(DEBUG, listW.empty(), "The worker "<<lastWorkerId<<" was deleted!");

  std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", lastWorkerId));
  sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
  ptrScheduler->addWorker(lastWorkerId, 1, cpbSet);

  LOG(DEBUG, "The worker "<<lastWorkerId<<" was re-added!");
  ptrScheduler->assignJobsToWorkers();
  ptrScheduler->checkAllocations();
  ptrScheduler->printAllocationTable();

  jobId = ptrScheduler->getAssignedJob(lastWorkerId);
  BOOST_CHECK(jobId==oldJobId);
  LOG_IF(DEBUG, jobId==oldJobId, "The worker "<<lastWorkerId<<" was re-assigned the job "<<jobId);
}

BOOST_AUTO_TEST_SUITE_END()
