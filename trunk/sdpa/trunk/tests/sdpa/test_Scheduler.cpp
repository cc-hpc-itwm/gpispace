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
#define BOOST_TEST_MODULE TestLoadBalancer
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>
#include <list>
#include <sdpa/memory.hpp>
#include <time.h>
#include <sdpa/util/util.hpp>
#include <fstream>

#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <tests/sdpa/DummyWorkflowEngine.hpp>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::daemon;

const int NWORKERS = 5;
const int NJOBS    = 20;

struct MyFixture
{
	MyFixture() :SDPA_INIT_LOGGER("sdpa.tests.testScheduler"){}
	~MyFixture(){}
	 SDPA_DECLARE_LOGGER();
};

BOOST_FIXTURE_TEST_SUITE( test_Scheduler, MyFixture )

BOOST_AUTO_TEST_CASE(testDelWorker)
{
	// first re-schedule the work:
	// inspect all queues and re-schedule each job
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create("orchestrator_0", "127.0.0.1:7000");

	ostringstream oss;
	sdpa::daemon::Scheduler::ptr_t ptr_scheduler_(new SchedulerImpl(ptrOrch.get()));
	ptr_scheduler_->start();

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

		ptrOrch->jobManager()->addJob(job_id, pJob);

		//add later preferences to the jobs
		ptr_scheduler_->schedule_remote(job_id);
	}

	ptr_scheduler_->getNextJob("Worker0","");
	ptr_scheduler_->getNextJob("Worker1","");

	for(int k=0; k<5; k++)
		ptr_scheduler_->getNextJob("Worker2","");

	ptr_scheduler_->getNextJob("Worker3","");
	ptr_scheduler_->getNextJob("Worker4","");

	SDPA_LOG_ERROR("Before re-scheduling ... ");
	ptr_scheduler_->print();

	// delete now the worker 2
	oss.str("");
	oss<<"Worker"<<2;
	string worker_id(oss.str());

	try
	{
		SDPA_LOG_ERROR("Delete the worker "<<worker_id<<" now ... ");
		ptr_scheduler_->delWorker(worker_id);
	}
	catch (const WorkerNotFoundException& ex)
	{
		SDPA_LOG_ERROR("Cannot delete the worker "<<worker_id<<". Worker not found!");
		throw ex;
	}

	SDPA_LOG_ERROR("After re-scheduling ... ");
	ptr_scheduler_->print();


	ptr_scheduler_->stop();

	sleep(1);
	SDPA_LOG_DEBUG("Worker deletion test finished!");
}

BOOST_AUTO_TEST_CASE(testSchedulerWithNoPrefs)
{
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create("orchestrator_0", "127.0.0.1:7000");

	ostringstream oss;
	sdpa::daemon::Scheduler::ptr_t ptr_scheduler_(new SchedulerImpl(ptrOrch.get()));
	ptr_scheduler_->start();

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

		 ptrOrch->jobManager()->addJob(job_id, pJob);

		 // add later preferences to the jobs
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
				 SDPA_LOG_DEBUG("The worker "<<workerId<<" was served the job "<<jobId.str() );
				 nJobsCompleted++;
			 }
			 catch( const NoJobScheduledException& ex )
			 {
				 SDPA_LOG_WARN("No job could be scheduled on the worker  "<<workerId );
			 }
		 }

	 SDPA_LOG_DEBUG("All "<<NJOBS<<" jobs were successfully executed!" );
	 ptr_scheduler_->stop();
}

BOOST_AUTO_TEST_CASE(testSchedulerWithPrefs)
{
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create("orchestrator_0", "127.0.0.1:7000");

	ostringstream oss;
	sdpa::daemon::Scheduler::ptr_t ptr_scheduler_(new SchedulerImpl(ptrOrch.get()));
	ptr_scheduler_->start();

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
		we::preference_t job_pref(true);
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
				 SDPA_LOG_DEBUG("The worker "<<workerId<<" was served the job "<<jobId.str() );
				 nJobsCompleted++;
			 }
			 catch( const NoJobScheduledException& ex )
			 {
				 SDPA_LOG_WARN("No job could be scheduled on the worker  "<<workerId );
			 }
		 }

	 SDPA_LOG_DEBUG("All "<<NJOBS<<" jobs were successfully executed!" );
	 ptr_scheduler_->stop();
}

/*
BOOST_AUTO_TEST_CASE(testSchedulerWithPrefsAndReScheduling)
{
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create("orchestrator_0", "127.0.0.1:7000");

	ostringstream oss;
	sdpa::daemon::Scheduler::ptr_t ptr_scheduler_(new SchedulerImpl(ptrOrch.get()));
	ptr_scheduler_->start();

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
		we::preference_t job_pref(true);
		job_pref.want( (i%NWORKERS + NWORKERS -1 )%NWORKERS );
		job_pref.want( i%NWORKERS );
		job_pref.want( (i%NWORKERS + 1)%NWORKERS );

		ptrOrch->jobManager()->addJob(job_id, pJob);
		ptrOrch->jobManager()->addJobPreferences(job_id, job_pref);

		//add later preferences to the jobs
		ptr_scheduler_->schedule_remote(job_id);
	 }

	 //delete a worker here

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
				 SDPA_LOG_DEBUG("The worker "<<workerId<<" was served the job "<<jobId.str() );
				 nJobsCompleted++;
			 }
			 catch( const NoJobScheduledException& ex )
			 {
				 SDPA_LOG_WARN("No job could be scheduled on the worker  "<<workerId );
			 }
		 }

	 SDPA_LOG_DEBUG("All "<<NJOBS<<" jobs were successfully executed!" );
	 ptr_scheduler_->stop();
}
*/

BOOST_AUTO_TEST_SUITE_END()
