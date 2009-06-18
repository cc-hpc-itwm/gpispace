#include "test_JobFSM.hpp"
#include <iostream>
#include "sdpa/jobFSM/JobFSM.hpp"
#include <string>
#include <list>

using namespace std;

using namespace sdpa::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( CJobFSMTest );

CJobFSMTest::CJobFSMTest()
    : SDPA_INIT_LOGGER("sdpa.tests.fsmTest")
{}

CJobFSMTest::~CJobFSMTest()
{}

void CJobFSMTest::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");
	m_JobFSM.initiate();
}

void CJobFSMTest::tearDown() { //stop the finite state machine
	SDPA_LOG_DEBUG("tearDown");
}

void CJobFSMTest::testJobFSM()
{
	list<sc::event_base*> listEvents;

	listEvents.push_back( new QueryJobStatusEvent(0));
	listEvents.push_back( new RunJobEvent(10));
	listEvents.push_back( new CancelJobEvent(10));
	listEvents.push_back( new CancelJobAckEvent(10));
	listEvents.push_back( new JobFinishedEvent(10));

	listEvents.push_back( new QueryJobStatusEvent(10));

	while( !listEvents.empty() )
	{
		sc::event_base* pEvt = listEvents.front();
		m_JobFSM.process_event(*pEvt);

		listEvents.pop_front();
		delete dynamic_cast<JobEvent*>(pEvt);
	}
}
