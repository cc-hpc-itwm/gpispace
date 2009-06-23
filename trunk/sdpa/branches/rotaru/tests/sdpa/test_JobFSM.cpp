#include "test_JobFSM.hpp"
#include <iostream>
#include "sdpa/jobFSM/JobFSM.hpp"
#include <string>
#include <list>
#include <sdpa/memory.hpp>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::events;

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
	list<sdpa::shared_ptr<sc::event_base> > listEvents;

	string strEmpty("");
	string strTen("10");

	QueryJobStatusEvent::Ptr pQueryJobStatusEvent(new QueryJobStatusEvent(strEmpty, strEmpty, strTen));
	listEvents.push_back(pQueryJobStatusEvent);

	RunJobEvent::Ptr pRunJobEvent(new RunJobEvent(strEmpty, strEmpty, strTen));
	listEvents.push_back(pRunJobEvent);

	CancelJobEvent::Ptr pCancelJobEvent(new CancelJobEvent(strEmpty, strEmpty, strTen));
	listEvents.push_back(pCancelJobEvent);

	CancelJobAckEvent::Ptr pCancelJobAckEvent(new CancelJobAckEvent(strEmpty, strEmpty, strTen));
	listEvents.push_back(pCancelJobAckEvent);

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
