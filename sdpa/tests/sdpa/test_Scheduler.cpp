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

BOOST_AUTO_TEST_CASE(testWorkStealing)
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
	requirement_list_t req_list_1;
	requirement_t req_1("C", true);
	req_list_1.push_back(req_1);
	pAgent->jobManager()->addJobRequirements(jobId1, req_list_1);

	LOG(INFO, "Schedule Job1 ...");
	ptrScheduler->schedule_with_constraints(jobId1);

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
	requirement_list_t req_list_2;
	requirement_t req_2("C", true);
	req_list_2.push_back(req_2);
	pAgent->jobManager()->addJobRequirements(jobId2, req_list_2);

	LOG(INFO, "Schedule Job2 ...");
	ptrScheduler->schedule_with_constraints(jobId2);
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

	// check if the two assigned workers are different
	BOOST_CHECK(workerId1!=workerId2);

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
	ptrScheduler->getNextJob(worker_B,"");

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
	requirement_list_t req_list_1;
	requirement_t req_1("C", true);
	req_list_1.push_back(req_1);
	pAgent->jobManager()->addJobRequirements(jobId1, req_list_1);

	LOG(INFO, "Schedule the job "<<jobId1);
	if(!ptrScheduler-> schedule_with_constraints(jobId1) )
	{
		LOG(INFO, "No matching worker found. Put the job "<<jobId1<<" into the common queue!");
		// do so as when no preferences were set, just ignore them right now
		ptrScheduler->schedule_anywhere(jobId1);
	}

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
	ptrScheduler->getNextJob(worker_A,"");

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

BOOST_AUTO_TEST_SUITE_END()
