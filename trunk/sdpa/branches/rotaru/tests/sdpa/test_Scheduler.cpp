#include "test_Scheduler.hpp"
#include <iostream>
#include <string>
#include <list>
#include <sdpa/memory.hpp>
#include <time.h>
#include <sdpa/util.hpp>
#include <fstream>
#include <sdpa/daemon/jobFSM/SMC/JobFSM.hpp>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::daemon;
using namespace sdpa::fsm::smc;


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

void SchedulerTest::testSchedulerImpl()
{
	 sdpa::daemon::Scheduler::ptr_t ptr_scheduler_(new SchedulerTestImpl());
	 ptr_scheduler_->start();
	for(int i=0; i<10; i++)
	{
		JobId job_id;
		Job::ptr_t pJob( new JobFSM( job_id, ""));
		pJob->set_local(true);
		ptr_scheduler_->schedule(pJob);
	}

	sleep(5);
	ptr_scheduler_->stop();
}
