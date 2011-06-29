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

#ifdef USE_REAL_WE
	#include <sdpa/daemon/nre/nre-worker/NreWorkerClient.hpp>
#else
	#include <sdpa/daemon/nre/BasicWorkerClient.hpp>
#endif

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::daemon;

const int NWORKERS = 5;
const int NJOBS    = 20;
const int MAX_CAP  = 100;

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
	sdpa::daemon::Scheduler::ptr_t ptr_scheduler_(new SchedulerImpl(ptrOrch.get(), false));

	// add a number of workers
	for( int k=0; k<NWORKERS; k++ )
	{
		oss.str("");
		oss<<"Worker"<<k;
		ptr_scheduler_->addWorker(oss.str(), k);
	}

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
		ptr_scheduler_->schedule_remote(job_id);
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
             sdpa::job_id_t jobId = ptr_scheduler_->getNextJob(workerId, "");
             LOG(DEBUG, "The worker "<<workerId<<" was served the job "<<jobId.str() );
             nJobsCompleted++;
         }
         catch( const NoJobScheduledException& ex )
         {
             LOG(WARN, "No job could be scheduled on the worker  "<<workerId );
         }
     }

   LOG(DEBUG, "All "<<NJOBS<<" jobs were successfully executed!" );
   seda::StageRegistry::instance().remove(ptrOrch->name());
}

BOOST_AUTO_TEST_SUITE_END()
