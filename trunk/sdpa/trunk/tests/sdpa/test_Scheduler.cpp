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

BOOST_AUTO_TEST_CASE(testDelWorker)
{
	// first re-schedule the work:
	// inspect all queues and re-schedule each job
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create("orchestrator_0", "127.0.0.1:7000");

	ostringstream oss;
	sdpa::daemon::Scheduler::ptr_t ptr_scheduler_(new SchedulerImpl);
	ptr_scheduler_->start(ptrOrch.get());

	boost::this_thread::sleep(boost::posix_time::seconds(1));

	LOG(INFO, "add a number of workers ...");
	for( int k=0; k<NWORKERS; k++ )
	{
		oss.str("");
		oss<<"Worker"<<k;
		ptr_scheduler_->addWorker(oss.str(), k);
	}

	LOG(INFO, "submit a number of remote jobs and schedule them");
	for(int i=0; i<NJOBS; i++)
	{
		JobId job_id;
		Job::ptr_t pJob( new JobFSM( job_id, ""));
		pJob->set_local(false);

		ptrOrch->jobManager()->addJob(job_id, pJob);

		LOG(INFO, "schedule remote the job "<<job_id);
		ptr_scheduler_->schedule_remote(job_id);
	}

	ptr_scheduler_->getNextJob("Worker0","");
	ptr_scheduler_->getNextJob("Worker1","");

	for(int k=0; k<5; k++)
		ptr_scheduler_->getNextJob("Worker2","");

	ptr_scheduler_->getNextJob("Worker3","");
	ptr_scheduler_->getNextJob("Worker4","");

	// delete now the worker 2
	oss.str("");
	oss<<"Worker"<<2;
	string worker_id(oss.str());

	LOG(ERROR, "Scheduler's content before deleting the worker "<<worker_id);
	ptr_scheduler_->print();

	try
	{
		LOG(ERROR, "Delete the worker "<<worker_id<<" now ... ");
		ptr_scheduler_->delWorker(worker_id);
	}
	catch (const WorkerNotFoundException& ex)
	{
		LOG(ERROR, "Cannot delete the worker "<<worker_id<<". Worker not found!");
		throw ex;
	}

	LOG(ERROR, "After re-scheduling ... ");
	ptr_scheduler_->print();

	ptr_scheduler_->stop();

	seda::StageRegistry::instance().remove(ptrOrch->name());
	boost::this_thread::sleep(boost::posix_time::seconds(1));
	LOG(INFO, "Worker deletion test finished!");
}

BOOST_AUTO_TEST_CASE(testSchedulerWithNoPrefs)
{
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create("orchestrator_0", "127.0.0.1:7000");

	ostringstream oss;
	sdpa::daemon::Scheduler::ptr_t ptr_scheduler_(new SchedulerImpl);
	ptr_scheduler_->start(ptrOrch.get());

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
				 LOG(DEBUG, "The worker "<<workerId<<" was served the job "<<jobId.str() );
				 nJobsCompleted++;
			 }
			 catch( const NoJobScheduledException& ex )
			 {
				 LOG(WARN, "No job could be scheduled on the worker  "<<workerId );
			 }
		 }

	 LOG(DEBUG, "All "<<NJOBS<<" jobs were successfully executed!" );
	 ptr_scheduler_->stop();
	 seda::StageRegistry::instance().remove(ptrOrch->name());
}

BOOST_AUTO_TEST_CASE(testSchedulerWithPrefs)
{
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create("orchestrator_0", "127.0.0.1:7000");

	ostringstream oss;
	sdpa::daemon::Scheduler::ptr_t ptr_scheduler_(new SchedulerImpl);
	ptr_scheduler_->start(ptrOrch.get());

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
	 ptr_scheduler_->stop();
	 seda::StageRegistry::instance().remove(ptrOrch->name());
}

BOOST_AUTO_TEST_SUITE_END()
