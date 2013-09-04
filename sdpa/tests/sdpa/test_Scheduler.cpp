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
using namespace sdpa::tests;
using namespace sdpa::daemon;

const int NWORKERS = 12;
const int NJOBS    = 4;
const int MAX_CAP  = 100;

const std::string WORKER_CPBS[] = {"CALC", "REDUCE", "MGMT"};

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

BOOST_AUTO_TEST_CASE(testSchedulerWithPrefs)
{
	string addrOrch = "127.0.0.1";
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);

	ostringstream oss;
	sdpa::daemon::Scheduler::ptr_t ptrScheduler(new SchedulerImpl(ptrOrch.get(), false));

	// add a number of workers
	for( int k=0; k<NWORKERS; k++ )
	{
		oss.str("");
		oss<<"Worker_"<<WORKER_CPBS[k/4]<<"_"<<k%4;

		sdpa::capabilities_set_t cpbSet;
		sdpa::worker_id_t workerId(oss.str());
		sdpa::capability_t cpb(WORKER_CPBS[k/4], "virtual", workerId);
		cpbSet.insert(cpb);
		ptrScheduler->addWorker(workerId, 1, cpbSet);
	}

	ptrScheduler->print();

	LOG(DEBUG, "All "<<NJOBS<<" jobs were successfully executed!" );
	seda::StageRegistry::instance().remove(ptrOrch->name());
}

/*BOOST_AUTO_TEST_CASE(testWorkStealing)
{
	string addrAg = "127.0.0.1";
	sdpa::master_info_list_t arrAgentMasterInfo;
	sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

	ostringstream oss;
	sdpa::daemon::SchedulerImpl::ptr_t ptrScheduler(new SchedulerImpl(pAgent.get(), false));

	sdpa::worker_id_t worker_A("worker_A");
	sdpa::worker_id_t worker_B("worker_B");

	//create 2 workers
	sdpa::capability_t cpb1("C", "virtual", worker_A);
	sdpa::capabilities_set_t cpbSetA;
	cpbSetA.insert(cpb1);
	ptrScheduler->addWorker(worker_A, 1, cpbSetA);

	sdpa::capability_t cpb2("C", "virtual", worker_B);
	sdpa::capabilities_set_t cpbSetB;
	cpbSetB.insert(cpb2);
	ptrScheduler->addWorker(worker_B, 1, cpbSetB);

	const sdpa::job_id_t jobId1("Job1");
	sdpa::daemon::Job::ptr_t pJob1(new JobFSM(jobId1, "description 1"));
	pAgent->jobManager()->addJob(jobId1, pJob1);
	job_requirements_t req_list_1;
	requirement_t req_1("C", true);
	req_list_1.add(req_1);
	pAgent->jobManager()->addJobRequirements(jobId1, req_list_1);

	LOG(INFO, "Schedule Job1 ...");
	ptrScheduler->schedule_remote(jobId1);

	// at this point the job jobId1 should be assigned to one of the workers
	// matching the requirements of jobId1, i.e. either worker_A or worker_B
	Worker::worker_id_t workerId1, workerId2;
	try {
		workerId1 = ptrScheduler->findWorker(jobId1);
		bool bInvariant = (workerId1=="worker_A" || workerId1=="worker_B");
		BOOST_CHECK(bInvariant);
		LOG(INFO, "The job Job1 was scheduled to "<<workerId1);
	}
	catch(WorkerNotFoundException& ex)
	{
		LOG(FATAL, "The job Job1 wasn't scheduled to any of the workers worker_A or worker_B");
	}

	const sdpa::job_id_t jobId2("Job2");
	sdpa::daemon::Job::ptr_t pJob2(new JobFSM(jobId2, "description 2"));
	pAgent->jobManager()->addJob(jobId2, pJob2);
	job_requirements_t req_list_2;
	requirement_t req_2("C", true);
	req_list_2.add(req_2);
	pAgent->jobManager()->addJobRequirements(jobId2, req_list_2);

	LOG(INFO, "Schedule Job2 ...");
	ptrScheduler->schedule_remote(jobId2);
	// at this point the job jobId2 should be assigned to one of the workers
	// that are matching the requirements of jobId2 and have no job assigned yet
	try {
		workerId2 = ptrScheduler->findWorker(jobId2);
		bool bInvariant = (workerId2=="worker_A" || workerId2=="worker_B");
		BOOST_CHECK(bInvariant);
		LOG(INFO, "The job Job2 was scheduled to "<<workerId2);
	}
	catch(WorkerNotFoundException& ex)
	{
		LOG(FATAL, "The job Job2 wasn't scheduled to any of the workers worker_A or worker_B");
	}

	// serve a job to the workerId1
	sdpa::job_id_t job1 = ptrScheduler->assignNewJob(workerId1,"");
	LOG(INFO, "The worker "<<workerId1<<" was served the job "<<job1);
	sdpa::job_id_t job2 = ptrScheduler->assignNewJob(workerId2,"");
	LOG(INFO, "The worker "<<workerId2<<" was served the job "<<job2);

	// check if the two assigned workers are different
	BOOST_CHECK(job1!=job2);

	LOG(INFO, "Delete the worker B now ...");
	ptrScheduler->delWorker(worker_B );

	// at this point the jobs assigned to worker_B should be re-distributed
    // to worker_A
	try {
		Worker::worker_id_t workerId = ptrScheduler->findWorker(jobId1);
		bool bInvariant = (workerId=="worker_A");
		BOOST_CHECK(bInvariant);
		LOG(INFO, "The job Job1 was scheduled to "<<workerId);
	}
	catch(WorkerNotFoundException& ex)
	{
		LOG(FATAL, "The job Job1 was supposed to be assigned or re-scheduled to worker_A!");
	}

	try {
		Worker::worker_id_t workerId = ptrScheduler->findWorker(jobId2);
		bool bInvariant = (workerId=="worker_A");
		BOOST_CHECK(bInvariant);
		LOG(INFO, "The job Job2 was scheduled to "<<workerId);
	}
	catch(WorkerNotFoundException& ex)
	{
		LOG(FATAL, "The job Job2 was supposed to be assigned or re-scheduled to worker_A!");
	}

	LOG(INFO, "Add the worker B again ...");
	ptrScheduler->addWorker(worker_B, 1, cpbSetB);

	LOG(INFO, "Get the next job assigned to the worker B ...");
	ptrScheduler->assignNewJob(worker_B,"");

	try {
		Worker::worker_id_t workerId = ptrScheduler->findWorker(jobId1);
		bool bInvariant = (workerId=="worker_A" || workerId=="worker_B");
		BOOST_CHECK(bInvariant);
		LOG(INFO, "The job Job1 was scheduled to "<<workerId);
	}
	catch(WorkerNotFoundException& ex)
	{
		LOG(FATAL, "The job Job1 was supposed to be assigned to one of the workers A or B!");
	}

	try {
		Worker::worker_id_t workerId = ptrScheduler->findWorker(jobId2);
		bool bInvariant = (workerId=="worker_A" || workerId=="worker_B");
		BOOST_CHECK(bInvariant);
		LOG(INFO, "The job Job2 was scheduled to "<<workerId);
	}
	catch(WorkerNotFoundException& ex)
	{
		LOG(FATAL, "The job Job2 was supposed to be assigned to one of the workers A or B!");
	}

   seda::StageRegistry::instance().remove(pAgent->name());
}
*/

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
	ptrScheduler->assignNewJob(worker_A, "");

	bOutcome = false;
	try {
		workerId1 = ptrScheduler->findWorker(jobId1);
		bOutcome = true;
		LOG(INFO, "The job Job1 was scheduled on worker_A, which is correct, because worker_A has now the required capability \"C\"");
	}
	catch(NoWorkerFoundException& ex)
	{
		bOutcome = false;
		LOG(INFO, "The job Job1 wasn't scheduled on worker_A, despite the fact is is the only one having the required  capability, which is incorrect");
	}

	// check if the two assigned workers are different
	BOOST_CHECK(bOutcome);
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
	ptrScheduler->getListWorkersNotFull(workerList);

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
	ptrScheduler->getListWorkersNotFull(workerList);
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
	ptrScheduler->getListWorkersNotFull(workerList);

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
	ptrScheduler->getListWorkersNotFull(workerList);
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
	ptrScheduler->getListWorkersNotFull(workerList);

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
	ptrScheduler->getListWorkersNotFull(workerList);
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
	ptrScheduler->getListWorkersNotFull(workerList);

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
	ptrScheduler->getListWorkersNotFull(workerList);
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
