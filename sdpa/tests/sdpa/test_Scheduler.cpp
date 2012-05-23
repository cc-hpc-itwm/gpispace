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
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
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

#include <sdpa/engine/DummyWorkflowEngine.hpp>
#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include <sdpa/engine/RealWorkflowEngine.hpp>


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

/*
	// submit a number of remote jobs and schedule them
	for(int i=0; i<NJOBS; i++)
	{
		JobId job_id;
		Job::ptr_t pJob( new JobFSM( job_id, ""));
		pJob->set_local(false);

		// specify some preferences for this job
		// job i prefers (i%NWORKERS + NWORKERS -1 )%NWORKERS, i%NWORKERS, (i%NWORKERS + 1)%NWORKERS,
		// mandatory
		preference_t job_pref(true);
		job_pref.want( (i%NWORKERS + NWORKERS -1 )%NWORKERS );
		job_pref.want( i%NWORKERS );
		job_pref.want( (i%NWORKERS + 1)%NWORKERS );

		ptrOrch->jobManager()->addJob(job_id, pJob);
		ptrOrch->jobManager()->addJobPreferences(job_id, job_pref);

		//add later preferences to the jobs
		ptrScheduler->schedule_remote(job_id);
	}

	// the workers request jobs
	int nJobsCompleted = 0;
	while( nJobsCompleted<NJOBS )
     for( int k=0; k<NWORKERS; k++ )
     {
         oss.str("");
         oss<<"Worker"<<k;
         Worker::worker_id_t workerId(oss.str());

         try {
             sdpa::job_id_t jobId = ptrScheduler->getNextJob(workerId, "");
             LOG(DEBUG, "The worker "<<workerId<<" was served the job "<<jobId.str() );
             nJobsCompleted++;
         }
         catch( const NoJobScheduledException& ex )
         {
             LOG(WARN, "No job could be scheduled on the worker  "<<workerId );
         }
     }
*/

   LOG(DEBUG, "All "<<NJOBS<<" jobs were successfully executed!" );
   seda::StageRegistry::instance().remove(ptrOrch->name());
}

BOOST_AUTO_TEST_SUITE_END()
