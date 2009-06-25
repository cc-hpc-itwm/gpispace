#include "test_JobFSM_BSC.hpp"
#include <iostream>
#include <string>
#include <list>
#include <sdpa/memory.hpp>
#include <sdpa/util.hpp>
#include <fstream>
#include <time.h>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::events;

CPPUNIT_TEST_SUITE_REGISTRATION( JobFSMTest_BSC );

JobFSMTest_BSC::JobFSMTest_BSC()
    : SDPA_INIT_LOGGER("sdpa.tests.JobFSMTest_BSC")
{}

JobFSMTest_BSC::~JobFSMTest_BSC()
{}

void JobFSMTest_BSC::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");
	m_JobFSM.initiate();
}

void JobFSMTest_BSC::tearDown() { //stop the finite state machine
	SDPA_LOG_DEBUG("tearDown");
}

void JobFSMTest_BSC::testJobFSM_BSC()
{
	list<sdpa::shared_ptr<sc::event_base> > listEvents;

	string strEmpty("");
	string strTen("10");

	sdpa::util::time_type start(sdpa::util::now());

	for(int k=0;k<1;k++)
	{
		QueryJobStatusEvent::Ptr pQueryJobStatusEvent(new QueryJobStatusEvent(strEmpty, strEmpty, strTen));
		listEvents.push_back(pQueryJobStatusEvent);

		RunJobEvent::Ptr pRunJobEvent(new RunJobEvent(strEmpty, strEmpty, strTen));
		listEvents.push_back(pRunJobEvent);

		JobFinishedEvent::Ptr pJobFinishedEvent(new JobFinishedEvent(strEmpty, strEmpty, strTen));
		listEvents.push_back(pJobFinishedEvent);

		QueryJobStatusEvent::Ptr pQueryJobStatusEvent2(new QueryJobStatusEvent(strEmpty, strEmpty, strTen));
		listEvents.push_back(pQueryJobStatusEvent2);

		while( !listEvents.empty() )
		{
			sdpa::shared_ptr<sc::event_base> pEvt = listEvents.front();
			m_JobFSM.process_event(*pEvt);

			listEvents.pop_front();
		}
	}

	sdpa::util::time_type delta(sdpa::util::time_diff(start, sdpa::util::now()));
	std::cout<< "bsc: " << delta << "us" << std::endl;
}
