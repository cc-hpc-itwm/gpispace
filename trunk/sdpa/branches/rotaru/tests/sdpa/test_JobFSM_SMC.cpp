#include "test_JobFSM_SMC.hpp"
#include <iostream>
#include <string>
#include <list>
#include <sdpa/memory.hpp>
#include <time.h>
#include <sdpa/util.hpp>
#include <fstream>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::events;

CPPUNIT_TEST_SUITE_REGISTRATION( JobFSMTest_SMC );

JobFSMTest_SMC::JobFSMTest_SMC() : SDPA_INIT_LOGGER("sdpa.tests.JobFSMTest_SMC"), m_JobFSM("10", "empty workflow")
{}

JobFSMTest_SMC::~JobFSMTest_SMC()
{}

void JobFSMTest_SMC::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");
}

void JobFSMTest_SMC::tearDown() { //stop the finite state machine
	SDPA_LOG_DEBUG("tearDown");
}

void JobFSMTest_SMC::testJobFSM_SMC()
{
	list<sdpa::shared_ptr<sc::event_base> > listEvents;

	string strFrom("");
	string strTo("");
	string strJobID = m_JobFSM.id().str();

    sdpa::util::time_type start(sdpa::util::now());

	/*m_JobFSM.GetContext().QueryJobStatus();

	//dispatch job
	m_JobFSM.GetContext().Dispatch();

	m_JobFSM.GetContext().JobFinished();

	m_JobFSM.GetContext().QueryJobStatus();

	m_JobFSM.GetContext().DeleteJob();*/
}
