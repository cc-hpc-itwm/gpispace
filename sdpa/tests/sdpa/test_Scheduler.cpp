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
#include <sdpa/daemon/JobFSM.hpp>
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
#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/agent/Agent.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/get_pointer.hpp>

using namespace std;
using namespace sdpa::daemon;

const int NWORKERS = 12;
const int NJOBS    = 4;
const int MAX_CAP  = 100;

const std::string WORKER_CPBS[] = {"A", "B", "C"};

typedef std::map<sdpa::job_id_t, sdpa::worker_id_t> mapJob2Worker_t;

struct MyFixture
{
    MyFixture() { FHGLOG_SETUP();}
    ~MyFixture()
    {
            seda::StageRegistry::instance().stopAll();
            seda::StageRegistry::instance().clear();
    }
};

BOOST_FIXTURE_TEST_SUITE( test_Scheduler, MyFixture )

BOOST_AUTO_TEST_CASE(testCoAllocation)
{
	LOG(INFO, "Test the co-allocation ...");

	string addrAg = "127.0.0.1";
	string strBackupOrch;
	ostringstream oss;

	sdpa::master_info_list_t arrAgentMasterInfo;
	sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

	pAgent-> createScheduler(false);

    if(!pAgent->scheduler())
    	LOG(FATAL, "The scheduler was not properly initialized");

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
		pAgent->scheduler()->addWorker(workerId, 1, cpbSet);
	}

	// create a number of jobs
	const sdpa::job_id_t jobId0("Job0");
	sdpa::daemon::Job::ptr_t pJob0(new JobFSM(jobId0, "description 0"));
	pAgent->jobManager()->addJob(jobId0, pJob0);

	const sdpa::job_id_t jobId1("Job1");
	sdpa::daemon::Job::ptr_t pJob1(new JobFSM(jobId1, "description 1"));
	pAgent->jobManager()->addJob(jobId1, pJob1);

	const sdpa::job_id_t jobId2("Job2");
	sdpa::daemon::Job::ptr_t pJob2(new JobFSM(jobId2, "description 2"));
	pAgent->jobManager()->addJob(jobId2, pJob2);

	job_requirements_t req_list_0;
	requirement_t req_0(WORKER_CPBS[0], true);
	req_list_0.add(req_0);
	req_list_0.setNumberOfInstances(4);
	pAgent->jobManager()->addJobRequirements(jobId0, req_list_0);
	pAgent->scheduler()->schedule_remote(jobId0);

	job_requirements_t req_list_1;
	requirement_t req_1(WORKER_CPBS[1], true);
	req_list_1.add(req_1);
	req_list_1.setNumberOfInstances(4);
	pAgent->jobManager()->addJobRequirements(jobId1, req_list_1);
	pAgent->scheduler()->schedule_remote(jobId1);

	job_requirements_t req_list_2;
	requirement_t req_2(WORKER_CPBS[2], true);
	req_list_2.add(req_2);
	req_list_2.setNumberOfInstances(4);
	pAgent->jobManager()->addJobRequirements(jobId2, req_list_2);
	pAgent->scheduler()->schedule_remote(jobId2);

	pAgent->scheduler()->assignJobsToWorkers();

	ostringstream ossrw;int k=-1;
	sdpa::worker_id_list_t listJobAssignedWorkers = pAgent->scheduler()->getListAllocatedWorkers(jobId0);
	BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
	{
		k = boost::lexical_cast<int>(wid);
		BOOST_CHECK( k==0 || k==3 || k==6 || k==9);
		ossrw<<wid<<" ";
	}
	//LOG(INFO, "The job jobId0 has been allocated the workers "<<ossrw.str());

	ossrw.str(""); listJobAssignedWorkers.clear();
	listJobAssignedWorkers = pAgent->scheduler()->getListAllocatedWorkers(jobId1);
	BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
	{
		k = boost::lexical_cast<int>(wid);
		BOOST_CHECK( k==1 || k==4 || k==7 || k==10);
		ossrw<<wid<<" ";
	}
	//LOG(INFO, "The job jobId1 has been allocated the workers "<<ossrw.str());

	ossrw.str(""); listJobAssignedWorkers.clear();
	listJobAssignedWorkers = pAgent->scheduler()->getListAllocatedWorkers(jobId2);
	BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
	{
		k = boost::lexical_cast<int>(wid);
		BOOST_CHECK( k==2 || k==5 || k==8 || k==11);
		ossrw<<wid<<" ";
	}
	//LOG(INFO, "The job jobId2 has been allocated the workers "<<ossrw.str());

	// try now to schedule a job requiring 2 resources of type "A"
	const sdpa::job_id_t jobId4("Job4");
	sdpa::daemon::Job::ptr_t pJob4(new JobFSM(jobId4, "description 4"));
	pAgent->jobManager()->addJob(jobId4, pJob4);

	req_list_0.setNumberOfInstances(2);
    pAgent->jobManager()->addJobRequirements(jobId4, req_list_0);
	pAgent->scheduler()->schedule_remote(jobId4);

	pAgent->scheduler()->assignJobsToWorkers();
	sdpa::worker_id_list_t listFreeWorkers(pAgent->scheduler()->getListAllocatedWorkers(jobId4));
	BOOST_CHECK(listFreeWorkers.empty());

	// Now report that jobId0 has finished and try to assign again resources to the job 4
	pAgent->scheduler()->releaseAllocatedWorkers(jobId0);
	listFreeWorkers.clear();
	pAgent->scheduler()->assignJobsToWorkers();
	listFreeWorkers = pAgent->scheduler()->getListAllocatedWorkers(jobId4);
	BOOST_CHECK(!listFreeWorkers.empty());

	int w0 = boost::lexical_cast<int>(listFreeWorkers[0]);
	BOOST_CHECK(w0==0 || w0 == 3  || w0 == 6|| w0 == 9);

	int w1 = boost::lexical_cast<int>(listFreeWorkers[1]);
	BOOST_CHECK(w1==0 || w1 == 3  || w1 == 6|| w1 == 9);
	//reinterpret_cast<SchedulerImpl*>(pAgent->scheduler().get())->printAllocationTable();
}


BOOST_AUTO_TEST_CASE(testGainCap)
{
	LOG(INFO, "Test scheduling when the required capabilities are gained later ...");
	string addrAg = "127.0.0.1";
	sdpa::master_info_list_t arrAgentMasterInfo;
	sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

	ostringstream oss;
	pAgent-> createScheduler(false);

	sdpa::daemon::Scheduler::ptr_t ptrScheduler = pAgent->scheduler();

	if(!ptrScheduler)
		LOG(FATAL, "The scheduler was not properly initialized");

	sdpa::worker_id_t worker_A("worker_A");

	sdpa::capabilities_set_t cpbSetA;
	ptrScheduler->addWorker(worker_A, 1, cpbSetA);

	const sdpa::job_id_t jobId1("Job1");
	sdpa::daemon::Job::ptr_t pJob1(new JobFSM(jobId1, "description 1"));
	pAgent->jobManager()->addJob(jobId1, pJob1);
	job_requirements_t req_list_1;
	requirement_t req_1("C", true);
	req_list_1.add(req_1);
	pAgent->jobManager()->addJobRequirements(jobId1, req_list_1);

	LOG(DEBUG, "Schedule the job "<<jobId1);
	ptrScheduler->schedule_remote(jobId1);

	ptrScheduler->assignJobsToWorkers();

	sdpa::worker_id_list_t listAssgnWks = pAgent->scheduler()->getListAllocatedWorkers(jobId1);
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

	listAssgnWks = pAgent->scheduler()->getListAllocatedWorkers(jobId1);
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
	string addrAg = "127.0.0.1";
	sdpa::master_info_list_t arrAgentMasterInfo;
	sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

	ostringstream oss;
	pAgent-> createScheduler(false);

	sdpa::daemon::SchedulerImpl::ptr_t ptrScheduler = boost::dynamic_pointer_cast<sdpa::daemon::SchedulerImpl>(pAgent->scheduler());

	if(!ptrScheduler)
		LOG(FATAL, "The scheduler was not properly initialized");

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
		sdpa::daemon::Job::ptr_t pJob(new JobFSM(jobId, ""));
		pAgent->jobManager()->addJob(jobId, pJob);

		job_requirements_t job_reqs;
		job_reqs.add(requirement_t("C", true));
		pAgent->jobManager()->addJobRequirements(jobId, job_reqs);
	}

	// schedule all jobs now
	BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
	{
		ptrScheduler->schedule_remote(jobId);
	}

	ptrScheduler->assignJobsToWorkers();

	sdpa::worker_id_list_t workerList;
	ptrScheduler->getListNotAllocatedWorkers(workerList);

	// check if there are any workers that are not yet reserved
	BOOST_CHECK(workerList.empty());

	BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
	{
		sdpa::worker_id_list_t listJobAssignedWorkers = pAgent->scheduler()->getListAllocatedWorkers(jobId);

		BOOST_CHECK(listJobAssignedWorkers.size() <= 1);
		if(!listJobAssignedWorkers.empty())
		{
			// delete the job
			pAgent->scheduler()->deleteWorkerJob(listJobAssignedWorkers[0], jobId);
		}
	}

	ptrScheduler->assignJobsToWorkers();

	workerList.clear();
	ptrScheduler->getListNotAllocatedWorkers(workerList);

	// check if the list of reserved workers is NJOBS - NWORKERS
	BOOST_CHECK_EQUAL(workerList.size(), 5);
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerJoinsLater)
{
	LOG(INFO, "Test the load-balancing when a worker joins later ...");

	string addrAg = "127.0.0.1";
	sdpa::master_info_list_t arrAgentMasterInfo;
	sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

	ostringstream oss;
	pAgent-> createScheduler(false);

	sdpa::daemon::SchedulerImpl::ptr_t ptrScheduler = boost::dynamic_pointer_cast<sdpa::daemon::SchedulerImpl>(pAgent->scheduler());

	if(!ptrScheduler)
		LOG(FATAL, "The scheduler was not properly initialized");


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
		sdpa::daemon::Job::ptr_t pJob(new JobFSM(jobId, ""));
		pAgent->jobManager()->addJob(jobId, pJob);
		job_requirements_t job_reqs;
		job_reqs.add(requirement_t("C", true));
		pAgent->jobManager()->addJobRequirements(jobId, job_reqs);
	}

	// schedule all jobs now
	BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
	{
		ptrScheduler->schedule_remote(jobId);
	}

	ptrScheduler->assignJobsToWorkers();

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

	ptrScheduler->assignJobsToWorkers();
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

	string addrAg = "127.0.0.1";
	sdpa::master_info_list_t arrAgentMasterInfo;
	sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

	ostringstream oss;
	pAgent-> createScheduler(false);

	sdpa::daemon::SchedulerImpl::ptr_t ptrScheduler = boost::dynamic_pointer_cast<sdpa::daemon::SchedulerImpl>(pAgent->scheduler());

	if(!ptrScheduler)
		LOG(FATAL, "The scheduler was not properly initialized");


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
		sdpa::daemon::Job::ptr_t pJob(new JobFSM(jobId, ""));
		pAgent->jobManager()->addJob(jobId, pJob);

		job_requirements_t job_reqs;
		job_reqs.add(requirement_t("C", true));
		pAgent->jobManager()->addJobRequirements(jobId, job_reqs);
	}

	// schedule all jobs now
	BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
	{
		ptrScheduler->schedule_remote(jobId);
	}

	ptrScheduler->assignJobsToWorkers();

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
	ptrScheduler->assignJobsToWorkers();
	workerList.clear();
	ptrScheduler->getListNotAllocatedWorkers(workerList);
	// all workers should be assigned a job, including the last one
	BOOST_CHECK(workerList.empty());
	//ptrScheduler->printAllocationTable();
}

BOOST_AUTO_TEST_CASE(tesLBStopRestartWorker)
{
	LOG(INFO, "Test the load-balancing when a worker is stopped, re-started and announces afterwards its capabilities ...");

	string addrAg = "127.0.0.1";
	sdpa::master_info_list_t arrAgentMasterInfo;
	sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

	ostringstream oss;
	pAgent-> createScheduler(false);

	sdpa::daemon::SchedulerImpl::ptr_t ptrScheduler = boost::dynamic_pointer_cast<sdpa::daemon::SchedulerImpl>(pAgent->scheduler());

	if(!ptrScheduler)
		LOG(FATAL, "The scheduler was not properly initialized");

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
		sdpa::daemon::Job::ptr_t pJob(new JobFSM(jobId, ""));
		pAgent->jobManager()->addJob(jobId, pJob);
		job_requirements_t job_reqs;
		job_reqs.add(requirement_t("C", true));
		pAgent->jobManager()->addJobRequirements(jobId, job_reqs);
	}

	// schedule all jobs now
	BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
	{
		ptrScheduler->schedule_remote(jobId);
	}

	ptrScheduler->assignJobsToWorkers();

	// all the workers should have assigned jobs
	sdpa::worker_id_list_t workerList;
	ptrScheduler->getListNotAllocatedWorkers(workerList);
	// check if there are any workers that are not yet reserved
	BOOST_CHECK(workerList.empty());

	sdpa::worker_id_t lastWorkerId("worker_9");
	sdpa::job_id_t jobId = ptrScheduler->getAssignedJob(lastWorkerId);
	LOG(DEBUG, "The worker "<<lastWorkerId<<" was assigned the job "<<jobId);

	// and now simply delete the last worker !!!!
	ptrScheduler->delWorker(lastWorkerId);
    sdpa::worker_id_list_t listW = ptrScheduler->getListAllocatedWorkers(jobId);
	BOOST_CHECK(listW.empty());
	LOG(DEBUG, "The worker "<<lastWorkerId<<" was deleted!");

	std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", lastWorkerId));
	sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
	// add now, add again the last worker
	ptrScheduler->addWorker(lastWorkerId, 1, cpbSet);

	LOG(DEBUG, "The worker "<<lastWorkerId<<" was re-added!");
	// assign jobs to the workers
	ptrScheduler->assignJobsToWorkers();
	sdpa::job_id_t oldJobId(jobId);
	jobId = ptrScheduler->getAssignedJob(lastWorkerId);
	BOOST_CHECK(jobId==oldJobId);
	if(jobId==oldJobId)
		LOG(DEBUG, "The worker "<<lastWorkerId<<" was re-assigned the job "<<jobId);
}

BOOST_AUTO_TEST_SUITE_END()
