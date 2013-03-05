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

#include <sdpa/engine/EmptyWorkflowEngine.hpp>

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

BOOST_AUTO_TEST_SUITE_END()
