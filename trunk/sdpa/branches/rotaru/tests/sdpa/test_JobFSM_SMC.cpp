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

JobFSMTest_SMC::JobFSMTest_SMC()
    : SDPA_INIT_LOGGER("sdpa.tests.JobFSMTest_SMC")
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

	string strEmpty("");
	string strTen("10");

    sdpa::util::time_type start(sdpa::util::now());

	for(int k=0;k<1;k++)
	{
		QueryJobStatusEvent evtQuery(strEmpty, strEmpty, strTen);
		m_JobFSM.GetContext().QueryJobStatus(evtQuery);

		RunJobEvent evtRun(strEmpty, strEmpty, strTen);
		m_JobFSM.GetContext().RunJob(evtRun);

		JobFinishedEvent evtFinished(strEmpty, strEmpty, strTen);
		m_JobFSM.GetContext().JobFinished(evtFinished);

		m_JobFSM.GetContext().QueryJobStatus(evtQuery);
	}

	sdpa::util::time_type delta(sdpa::util::time_diff(start, sdpa::util::now()));
	std::cout << "smc: " << delta << "us" << std::endl;
}
