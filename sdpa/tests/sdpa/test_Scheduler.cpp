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
const int NJOBS    = 20;
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

	const sdpa::job_id_t jobId2("Job2");
	sdpa::daemon::Job::ptr_t pJob2(new JobFSM(jobId2, "description 2"));
	pAgent->jobManager()->addJob(jobId2, pJob2);
	requirement_list_t req_list_2;
	requirement_t req_2("C", true);
	req_list_2.push_back(req_2);
	pAgent->jobManager()->addJobRequirements(jobId2, req_list_2);

	LOG(INFO, "Schedule Job2 ...");
	ptrScheduler->schedule_with_constraints(jobId2);

	//sleep(1);

	ptrScheduler->print();
	LOG(INFO, "Delete the worker B now ...");
	ptrScheduler->delWorker(worker_B );
	ptrScheduler->print();

	//sleep(1);
	LOG(INFO, "Add the worker B again ...");
	ptrScheduler->addWorker(worker_B, 1, cpbSetB);
	//sleep(1);

	LOG(INFO, "Get the next job assigned to the worker B ...");
	ptrScheduler->getNextJob(worker_B,"");

	ptrScheduler->print();

	//sleep(5);

   seda::StageRegistry::instance().remove(pAgent->name());
}

BOOST_AUTO_TEST_SUITE_END()
