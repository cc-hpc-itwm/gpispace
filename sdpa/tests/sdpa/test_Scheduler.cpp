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
	req_list_0.n_workers_req = 4;
	pAgent->jobManager()->addJobRequirements(jobId0, req_list_0);
	pAgent->scheduler()->schedule_remote(jobId0);

	job_requirements_t req_list_1;
	requirement_t req_1(WORKER_CPBS[1], true);
	req_list_1.add(req_1);
	req_list_1.n_workers_req = 4;
	pAgent->jobManager()->addJobRequirements(jobId1, req_list_1);
	pAgent->scheduler()->schedule_remote(jobId1);

	job_requirements_t req_list_2;
	requirement_t req_2(WORKER_CPBS[2], true);
	req_list_2.add(req_2);
	req_list_2.n_workers_req = 4;
	pAgent->jobManager()->addJobRequirements(jobId2, req_list_2);
	pAgent->scheduler()->schedule_remote(jobId2);

	pAgent->scheduler()->assignWorkersToJobs();

	ostringstream ossrw;int k=-1;
	sdpa::worker_id_list_t listJobAssignedWorkers = pAgent->scheduler()->getListReservedWorkers(jobId0);
	BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
	{
		k = boost::lexical_cast<int>(wid);
		BOOST_CHECK( k==0 || k==3 || k==6 || k==9);
		ossrw<<wid<<" ";
	}
	//LOG(INFO, "The job jobId0 has been allocated the workers "<<ossrw.str());

	ossrw.str(""); listJobAssignedWorkers.clear();
	listJobAssignedWorkers = pAgent->scheduler()->getListReservedWorkers(jobId1);
	BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
	{
		k = boost::lexical_cast<int>(wid);
		BOOST_CHECK( k==1 || k==4 || k==7 || k==10);
		ossrw<<wid<<" ";
	}
	//LOG(INFO, "The job jobId1 has been allocated the workers "<<ossrw.str());

	ossrw.str(""); listJobAssignedWorkers.clear();
	listJobAssignedWorkers = pAgent->scheduler()->getListReservedWorkers(jobId2);
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

	req_list_0.n_workers_req = 2;
    pAgent->jobManager()->addJobRequirements(jobId4, req_list_0);
	pAgent->scheduler()->schedule_remote(jobId4);

	pAgent->scheduler()->assignWorkersToJobs();
	sdpa::worker_id_list_t listFreeWorkers(pAgent->scheduler()->getListReservedWorkers(jobId4));
	BOOST_CHECK(listFreeWorkers.empty());

	// Now report that jobId0 has finished and try to assign again resources to the job 4
	pAgent->scheduler()->releaseAllocatedWorkers(jobId0);
	listFreeWorkers.clear();
	pAgent->scheduler()->assignWorkersToJobs();
	listFreeWorkers = pAgent->scheduler()->getListReservedWorkers(jobId4);
	BOOST_CHECK(!listFreeWorkers.empty());

	int w0 = boost::lexical_cast<int>(listFreeWorkers[0]);
	BOOST_CHECK(w0==0 || w0 == 3  || w0 == 6|| w0 == 9);

	int w1 = boost::lexical_cast<int>(listFreeWorkers[1]);
	BOOST_CHECK(w1==0 || w1 == 3  || w1 == 6|| w1 == 9);
	//reinterpret_cast<SchedulerImpl*>(pAgent->scheduler().get())->printAllocationTable();
}

BOOST_AUTO_TEST_CASE(testGainCap)
{
	LOG(INFO, "Test scheduling when the required cabilities are gained later ...");
	string addrAg = "127.0.0.1";
	sdpa::master_info_list_t arrAgentMasterInfo;
	sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

	ostringstream oss;
	sdpa::daemon::SchedulerImpl::ptr_t ptrScheduler(new SchedulerImpl(pAgent.get(), false));

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

	LOG(INFO, "Schedule the job "<<jobId1);
	ptrScheduler-> schedule_remote(jobId1);

	// at this point the job jobId1 should be assigned to one of the workers
	// matching the requirements of jobId1, i.e. either worker_A or worker_B
	Worker::worker_id_t workerId1, workerId2;
	bool bOutcome = false;
	try {
		workerId1 = ptrScheduler->findWorker(jobId1);
		bOutcome = false;
		LOG(INFO, "The job Job1 was scheduled on worker_A, which is incorrect, because worker_A doesn't have yet the capability \"C\"");
	}
	catch(NoWorkerFoundException& ex)
	{
		bOutcome = true;
		LOG(INFO, "The job Job1 wasn't scheduled on worker_A, which is correct, as it has not yet acquired the capability \"C\"");
	}

	BOOST_CHECK(bOutcome);

	sdpa::capability_t cpb1("C", "virtual", worker_A);
	cpbSetA.insert(cpb1);
	ptrScheduler->addCapabilities(worker_A, cpbSetA);

	LOG(INFO, "Check if worker_A really acquired the capability \"C\"");

	sdpa::capabilities_set_t cpbset;
	ptrScheduler->getWorkerCapabilities(worker_A, cpbset);

	LOG(INFO, "The worker_A has now the following capabilities: ["<<cpbset<<"]");

	LOG(INFO, "Get the next job assigned to the worker A ...");
	sdpa::job_id_t assgnJobId(ptrScheduler->assignNewJob(worker_A, ""));
	LOG(INFO, "To worker_A was assigned the job "<<assgnJobId<<"...");

	// check if the two assigned workers are different
	BOOST_CHECK_EQUAL(assgnJobId.str(), "Job1");
	if(assgnJobId=="Job1")
		LOG(INFO, "The job Job1 was scheduled on worker_A, which is correct, because worker_A has now the required capability \"C\"");
	else
		LOG(INFO, "The job Job1 wasn't scheduled on worker_A, despite the fact is is the only one having the required  capability, which is incorrect");
}

BOOST_AUTO_TEST_CASE(tesLoadBalancing)
{
	string addrAg = "127.0.0.1";
	sdpa::master_info_list_t arrAgentMasterInfo;
	sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

	ostringstream oss;
	sdpa::daemon::SchedulerImpl::ptr_t ptrScheduler(new SchedulerImpl(pAgent.get(), false));

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

	sdpa::worker_id_list_t workerList;
	ptrScheduler->getListNotFullWorkers(workerList);

    // first round: try serve all the workers a job
	mapJob2Worker_t mapJob2Worker;
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		try {
			sdpa::job_id_t jobId = ptrScheduler->assignNewJob(workerId, "");
			LOG(INFO, "The job "<<jobId<<" was assigned to "<<workerId);
			mapJob2Worker.insert(mapJob2Worker_t::value_type(jobId, workerId));
		}
		catch( const NoJobScheduledException& ex)
		{
			LOG(INFO, "There is no job to assign to the worker "<<workerId<<"  ...");
		}
	}

	// at this point, inspect the workers, all of them should be assigned exactly one job
	LOG(INFO, "Check if all the workers have exactly one job assigned ...");
	ptrScheduler->getWorkerList(workerList);
	bool bInvariant = true;
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		Worker::ptr_t ptrWorker(ptrScheduler->findWorker(workerId));
		bInvariant = bInvariant && (ptrWorker->nbAllocatedJobs()==1);
	}

	BOOST_CHECK(bInvariant);

	// announce the jobs as finished and delete them
	BOOST_FOREACH(const mapJob2Worker_t::value_type& pair, mapJob2Worker)
	{
		sdpa::job_id_t jobId = pair.first;
		sdpa::worker_id_t workerId = pair.second;

		// acknowledge the job
		LOG(INFO, "Acknowledge the "<<jobId<<" to the worker "<<workerId);
		ptrScheduler->acknowledgeJob(workerId, jobId);

		LOG(INFO, "Delete the job "<<jobId<<" (assigned to "<<workerId<<")");
		// acknowledge the job
		ptrScheduler->deleteWorkerJob(workerId, jobId);
	}

	// at this point, inspect the workers, none of them should have a job assigned
	ptrScheduler->getWorkerList(workerList);

	bInvariant = true;
	LOG(INFO, "Check if no worker has jobs assigned (all the jobs assigned in the first round are supposed to be finished) ...)");
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		Worker::ptr_t ptrWorker(ptrScheduler->findWorker(workerId));
		bInvariant = bInvariant && (ptrWorker->nbAllocatedJobs()==0);
	}

	BOOST_CHECK(bInvariant);

	// second round: serve the rest of the remaining jobs
	ptrScheduler->getListNotFullWorkers(workerList);
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		try {
			sdpa::job_id_t jobId = ptrScheduler->assignNewJob(workerId, "");
			LOG(INFO, "The job "<<jobId<<" was assigned to "<<workerId);
		}
		catch( const NoJobScheduledException& ex)
		{
			LOG(INFO, "There is no job to assign to the worker "<<workerId<<"  ...");
		}
	}

	// at this point, inspect the workers, all of them should be assigned at most one job
	LOG(INFO, "Check if all the workers have at most one job assigned ...");
	ptrScheduler->getWorkerList(workerList);
	bInvariant = true;
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		Worker::ptr_t ptrWorker(ptrScheduler->findWorker(workerId));
		LOG(INFO, "The worker "<<workerId<<" has "<<ptrWorker->nbAllocatedJobs()<<" jobs allocated!");
		bInvariant = bInvariant && (ptrWorker->nbAllocatedJobs()<=1);
	}

	BOOST_CHECK(bInvariant);
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerJoinsLater)
{
	LOG(INFO, "Test the load-balancing when a worker joins later ...");

	string addrAg = "127.0.0.1";
	sdpa::master_info_list_t arrAgentMasterInfo;
	sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

	ostringstream oss;
	sdpa::daemon::SchedulerImpl::ptr_t ptrScheduler(new SchedulerImpl(pAgent.get(), false));

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

	sdpa::worker_id_list_t workerList;
	ptrScheduler->getListNotFullWorkers(workerList);

    // first round: try serve all the workers a job
	mapJob2Worker_t mapJob2Worker;
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		try {
			sdpa::job_id_t jobId = ptrScheduler->assignNewJob(workerId, "");
			LOG(INFO, "The job "<<jobId<<" was assigned to "<<workerId);
			mapJob2Worker.insert(mapJob2Worker_t::value_type(jobId, workerId));
		}
		catch( const NoJobScheduledException& ex)
		{
			LOG(INFO, "There is no job to assign to the worker "<<workerId<<"  ...");
		}
	}

	// at this point, inspect the workers, all of them should have one job assigned!
	LOG(INFO, "Check if all the workers have exactly one job assigned ...");
	ptrScheduler->getWorkerList(workerList);
	bool bInvariant = true;
	Worker::ptr_t ptrWorker;
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		ptrWorker = ptrScheduler->findWorker(workerId);
		bInvariant = bInvariant && (ptrWorker->nbAllocatedJobs()==1);
	}

	BOOST_CHECK(bInvariant);

	LOG(INFO, "A new worker with the capability \"C\" joins ...");
	osstr<<"worker_"<<nWorkers-1;
	sdpa::worker_id_t lastWorkerId(osstr.str());
	osstr.str("");
	arrWorkerIds.push_back(lastWorkerId);
	std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", lastWorkerId));
	sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
	ptrScheduler->addWorker(lastWorkerId, 1, cpbSet);

	// try to assign a job to the last worker
	LOG(INFO, "Try to assign a job to the last worker ...");
	try {
		sdpa::job_id_t jobId = ptrScheduler->assignNewJob(lastWorkerId, "");
		LOG(INFO, "The job "<<jobId<<" was assigned to "<<lastWorkerId);
		mapJob2Worker.insert(mapJob2Worker_t::value_type(jobId, lastWorkerId));
	}
	catch( const NoJobScheduledException& ex)
	{
		LOG(INFO, "There is no job to assign to the worker "<<lastWorkerId<<"  ...");
	}

	// at this point, the last worker should have as well a job assigned!
	LOG(INFO, "Check if The last worker has assigned a job, too, now ...");
	ptrWorker = ptrScheduler->findWorker(lastWorkerId);
	BOOST_CHECK(ptrWorker->nbAllocatedJobs()==1);

	// consume the first wave of jobs
	BOOST_FOREACH(const mapJob2Worker_t::value_type& pair, mapJob2Worker)
	{
		sdpa::job_id_t jobId = pair.first;
		sdpa::worker_id_t workerId = pair.second;

		// acknowledge the job
		LOG(INFO, "Acknowledge the "<<jobId<<" to the worker "<<workerId);
		ptrScheduler->acknowledgeJob(workerId, jobId);

		LOG(INFO, "Delete the job "<<jobId<<" (assigned to "<<workerId<<")");
		// acknowledge the job
		ptrScheduler->deleteWorkerJob(workerId, jobId);
	}

	// second round: serve the rest of the remaining jobs
	LOG(INFO, "Schedule the rest of the jobs and ...");
	ptrScheduler->getListNotFullWorkers(workerList);
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		try {
			sdpa::job_id_t jobId = ptrScheduler->assignNewJob(workerId, "");
			LOG(INFO, "The job "<<jobId<<" was assigned to "<<workerId);
		}
		catch( const NoJobScheduledException& ex)
		{
			LOG(INFO, "There is no job to assign to the worker "<<workerId<<"  ...");
		}
	}

	// at this point, inspect the workers, all of them should be assigned at most one job
	LOG(INFO, "Check if all the workers have at most one job assigned ...");
	ptrScheduler->getWorkerList(workerList);
	bInvariant = true;
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		Worker::ptr_t ptrWorker(ptrScheduler->findWorker(workerId));
		LOG(INFO, "The worker "<<workerId<<" has "<<ptrWorker->nbAllocatedJobs()<<" jobs allocated!");
		bInvariant = bInvariant && (ptrWorker->nbAllocatedJobs()<=1);
	}

	BOOST_CHECK(bInvariant);
}

BOOST_AUTO_TEST_CASE(tesLBOneWorkerGainsCpbLater)
{
	LOG(INFO, "Test the load-balancing when a worker gains a capability later ...");

	string addrAg = "127.0.0.1";
	sdpa::master_info_list_t arrAgentMasterInfo;
	sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

	ostringstream oss;
	sdpa::daemon::SchedulerImpl::ptr_t ptrScheduler(new SchedulerImpl(pAgent.get(), false));

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

	sdpa::worker_id_list_t workerList;
	ptrScheduler->getListNotFullWorkers(workerList);

    // first round: try serve all the workers a job
	mapJob2Worker_t mapJob2Worker;
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		try {
			sdpa::job_id_t jobId = ptrScheduler->assignNewJob(workerId, "");
			LOG(INFO, "The job "<<jobId<<" was assigned to "<<workerId);
			mapJob2Worker.insert(mapJob2Worker_t::value_type(jobId, workerId));
		}
		catch( const NoJobScheduledException& ex)
		{
			LOG(INFO, "There is no job to assign to the worker "<<workerId<<", as it doesn't possess the required capability  ...");
		}
	}

	// at this point, inspect the workers, all of them should be assigned exactly one job
	// except, the last one who shouldn't have any job assigned!
	LOG(INFO, "Check if all the workers have exactly one job assigned, except the last one, who shouldn't have any ...");
	ptrScheduler->getWorkerList(workerList);
	bool bInvariant = true;
	Worker::ptr_t ptrWorker;
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		ptrWorker = ptrScheduler->findWorker(workerId);
		bInvariant = bInvariant && (ptrWorker->nbAllocatedJobs()==1);
	}

	bool bInvWorkerLateCpbs(!bInvariant);
	BOOST_CHECK(bInvWorkerLateCpbs);

	osstr<<"worker_"<<nWorkers-1;
	sdpa::worker_id_t lastWorkerId(osstr.str());
	osstr.str("");

	LOG(INFO, "The last worker (\""<<lastWorkerId<<"\") gains the capability \"C\" ...");

	LOG(INFO, "Check if the last worker really has no job assigned ...");
	ptrWorker = ptrScheduler->findWorker(lastWorkerId);
	bool bInvNoJobAssgnd(ptrWorker->nbAllocatedJobs()==0);
	BOOST_CHECK(bInvNoJobAssgnd);

	std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", lastWorkerId));
	sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
	ptrScheduler->addCapabilities(lastWorkerId, cpbSet);

	// try to assign a job to the last worker
	LOG(INFO, "Try to assign a job to the last worker ...");
	try {
		sdpa::job_id_t jobId = ptrScheduler->assignNewJob(lastWorkerId, "");
		LOG(INFO, "The job "<<jobId<<" was assigned to "<<lastWorkerId);
		mapJob2Worker.insert(mapJob2Worker_t::value_type(jobId, lastWorkerId));
	}
	catch( const NoJobScheduledException& ex)
	{
		LOG(INFO, "There is no job to assign to the worker "<<lastWorkerId<<" ...");
	}

	// at this point, inspect the workers, all of them should be assigned exactly one job
	// except, the last one who shouldn't have any job assigned!
	LOG(INFO, "Check if the last worker has a job assigned, too, now ...");
	ptrWorker = ptrScheduler->findWorker(lastWorkerId);
	BOOST_CHECK(ptrWorker->nbAllocatedJobs()==1);

	// consume the first wave of jobs
	BOOST_FOREACH(const mapJob2Worker_t::value_type& pair, mapJob2Worker)
	{
		sdpa::job_id_t jobId = pair.first;
		sdpa::worker_id_t workerId = pair.second;

		// acknowledge the job
		LOG(INFO, "Acknowledge the "<<jobId<<" to the worker "<<workerId);
		ptrScheduler->acknowledgeJob(workerId, jobId);

		LOG(INFO, "Delete the job "<<jobId<<" (assigned to "<<workerId<<")");
		// acknowledge the job
		ptrScheduler->deleteWorkerJob(workerId, jobId);
	}

	// second round: serve the rest of the remaining jobs
	LOG(INFO, "Schedule the rest of the jobs and ...");
	ptrScheduler->getListNotFullWorkers(workerList);
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		try {
			sdpa::job_id_t jobId = ptrScheduler->assignNewJob(workerId, "");
			LOG(INFO, "The job "<<jobId<<" was assigned to "<<workerId);
		}
		catch( const NoJobScheduledException& ex)
		{
			LOG(INFO, "There is no job to assign to the worker "<<workerId<<"  ...");
		}
	}

	// at this point, inspect the workers, all of them should be assigned at most one job
	LOG(INFO, "Check if all the workers have at most one job assigned ...");
	ptrScheduler->getWorkerList(workerList);
	bInvariant = true;
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		Worker::ptr_t ptrWorker(ptrScheduler->findWorker(workerId));
		LOG(INFO, "The worker "<<workerId<<" has "<<ptrWorker->nbAllocatedJobs()<<" jobs allocated!");
		bInvariant = bInvariant && (ptrWorker->nbAllocatedJobs()<=1);
	}

	BOOST_CHECK(bInvariant);
}

BOOST_AUTO_TEST_CASE(tesLBStopRestartWorker)
{
	LOG(INFO, "Test the load-balancing when a worker is stopped, re-started and announces afterwards its capabilities ...");

	string addrAg = "127.0.0.1";
	sdpa::master_info_list_t arrAgentMasterInfo;
	sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

	ostringstream oss;
	sdpa::daemon::SchedulerImpl::ptr_t ptrScheduler(new SchedulerImpl(pAgent.get(), false));

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

	sdpa::worker_id_list_t workerList;
	ptrScheduler->getListNotFullWorkers(workerList);

    // first round: try serve all the workers a job
	mapJob2Worker_t mapJob2Worker;
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		try {
			sdpa::job_id_t jobId = ptrScheduler->assignNewJob(workerId, "");
			LOG(INFO, "The job "<<jobId<<" was assigned to "<<workerId);
			mapJob2Worker.insert(mapJob2Worker_t::value_type(jobId, workerId));
		}
		catch( const NoJobScheduledException& ex)
		{
			LOG(INFO, "There is no job to assign to the worker "<<workerId<<", as it doesn't possess the required capability  ...");
		}
	}

	osstr<<"worker_"<<nWorkers-1;
	sdpa::worker_id_t lastWorkerId(osstr.str());
	osstr.str("");

	LOG(INFO, "One worker goes down now ...");
	ptrScheduler->delWorker(lastWorkerId);

	// at this point, inspect the workers, all of them should be assigned exactly one job
	// except, the last one who shouldn't have any job assigned!
	LOG(INFO, "Check if all the workers have exactly one job assigned ...");
	ptrScheduler->getWorkerList(workerList);
	bool bInvariant = true;
	Worker::ptr_t ptrWorker;
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		ptrWorker = ptrScheduler->findWorker(workerId);
		bInvariant = bInvariant && (ptrWorker->nbAllocatedJobs()==1);
	}

	BOOST_CHECK(bInvariant);

	LOG(INFO, "The worker "<<lastWorkerId<<" re-registers ...");
	ptrScheduler->addWorker(lastWorkerId, 1);

	LOG(INFO, "Check if the last worker really has no job assigned ...");
	ptrWorker = ptrScheduler->findWorker(lastWorkerId);
	bool bInvNoJobAssgnd(ptrWorker->nbAllocatedJobs()==0);
	BOOST_CHECK(bInvNoJobAssgnd);

	LOG(INFO, "The last worker gains the capability \"C\" now ...");
	std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", lastWorkerId));
	sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
	ptrScheduler->addCapabilities(lastWorkerId, cpbSet);

	// try to assign a job to the last worker
	LOG(INFO, "Try to assign a job to the last worker ...");
	try {
		sdpa::job_id_t jobId = ptrScheduler->assignNewJob(lastWorkerId, "");
		LOG(INFO, "The job "<<jobId<<" was assigned to "<<lastWorkerId);
		mapJob2Worker.insert(mapJob2Worker_t::value_type(jobId, lastWorkerId));
	}
	catch( const NoJobScheduledException& ex)
	{
		LOG(INFO, "There is no job to assign to the worker "<<lastWorkerId<<" ...");
	}

	// at this point, inspect the workers, all of them should be assigned exactly one job
	// except, the last one who shouldn't have any job assigned!
	LOG(INFO, "Check if the last worker has a job assigned, too, now ...");
	ptrWorker = ptrScheduler->findWorker(lastWorkerId);
	BOOST_CHECK(ptrWorker->nbAllocatedJobs()==1);

	// consume the first wave of jobs
	BOOST_FOREACH(const mapJob2Worker_t::value_type& pair, mapJob2Worker)
	{
		sdpa::job_id_t jobId = pair.first;
		sdpa::worker_id_t workerId = pair.second;

		// acknowledge the job
		LOG(INFO, "Acknowledge the "<<jobId<<" to the worker "<<workerId);
		ptrScheduler->acknowledgeJob(workerId, jobId);

		LOG(INFO, "Delete the job "<<jobId<<" (assigned to "<<workerId<<")");
		// acknowledge the job
		ptrScheduler->deleteWorkerJob(workerId, jobId);
	}

	// second round: serve the rest of the remaining jobs
	LOG(INFO, "Schedule the rest of the jobs and ...");
	ptrScheduler->getListNotFullWorkers(workerList);
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		try {
			sdpa::job_id_t jobId = ptrScheduler->assignNewJob(workerId, "");
			LOG(INFO, "The job "<<jobId<<" was assigned to "<<workerId);
		}
		catch( const NoJobScheduledException& ex)
		{
			LOG(INFO, "There is no job to assign to the worker "<<workerId<<"  ...");
		}
	}

	// at this point, inspect the workers, all of them should be assigned at most one job
	LOG(INFO, "Check if all the workers have at most one job assigned ...");
	ptrScheduler->getWorkerList(workerList);
	bInvariant = true;
	BOOST_FOREACH(const sdpa::worker_id_t& workerId, workerList)
	{
		Worker::ptr_t ptrWorker(ptrScheduler->findWorker(workerId));
		LOG(INFO, "The worker "<<workerId<<" has "<<ptrWorker->nbAllocatedJobs()<<" jobs allocated!");
		bInvariant = bInvariant && (ptrWorker->nbAllocatedJobs()<=1);
	}

	BOOST_CHECK(bInvariant);
}

BOOST_AUTO_TEST_SUITE_END()
