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
#include "test_Scheduler.hpp"
#include <iostream>
#include <string>
#include <list>
#include <sdpa/memory.hpp>
#include <time.h>
#include <sdpa/util/util.hpp>
#include <fstream>
#include <sdpa/daemon/jobFSM/SMC/JobFSM.hpp>
#include <sdpa/daemon/SchedulerImpl.hpp>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::daemon;
using namespace sdpa::fsm::smc;

const int NWORKERS = 5;
const int NJOBS    = 20;

CPPUNIT_TEST_SUITE_REGISTRATION( SchedulerTest );

SchedulerTest::SchedulerTest() : SDPA_INIT_LOGGER("sdpa.tests.SchedulerTest")
{

}

SchedulerTest::~SchedulerTest()
{
}

void SchedulerTest::setUp()
{
	SDPA_LOG_DEBUG("setUP");
	//initialize and start the finite state machine
}

void SchedulerTest::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
}

void SchedulerTest::testSchedulerWithNoPrefs()
{
	 ostringstream oss;
	 sdpa::daemon::Scheduler::ptr_t ptr_scheduler_(new SchedulerImpl());
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

		//add later preferences to the jobs

		ptr_scheduler_->schedule(job_id);
	 }

	 // the workers request jobs
	 /*int nJobsCompleted = 0;
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

	 SDPA_LOG_DEBUG("All "<<NJOBS<<" jobs were successfully executed!" );*/
	 ptr_scheduler_->stop();
}
