#include "test_OrchFSM_BSC.hpp"
#include <iostream>
#include <string>
#include <list>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::events;

CPPUNIT_TEST_SUITE_REGISTRATION( COrchFSMTest );

COrchFSMTest::COrchFSMTest()
    : SDPA_INIT_LOGGER("sdpa.tests.fsmTest")
{}

COrchFSMTest::~COrchFSMTest()
{}

void COrchFSMTest::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");
	m_OrchFSM.initiate();
}

void COrchFSMTest::tearDown() { //stop the finite state machine
	SDPA_LOG_DEBUG("tearDown");
}

void COrchFSMTest::testOrchFSM()
{
	/*list<sc::event_base*> listEvents;

	string strEmpty("");

	listEvents.push_back( new StartUpEvent(strEmpty,strEmpty));
	listEvents.push_back( new LifeSignEvent(strEmpty,strEmpty));
	listEvents.push_back( new RequestJobEvent(strEmpty,strEmpty));
	listEvents.push_back( new SubmitAckEvent(strEmpty,strEmpty));
	listEvents.push_back( new DeleteJobEvent(strEmpty,strEmpty));
	listEvents.push_back( new ConfigRequestEvent(strEmpty,strEmpty));
	listEvents.push_back( new InterruptEvent(strEmpty,strEmpty));

	while( !listEvents.empty() )
	{
		sc::event_base* pEvt = listEvents.front();
		m_OrchFSM.process_event(*pEvt);

		listEvents.pop_front();
		delete dynamic_cast<MgmtEvent*>(pEvt);
	}
	*/
}
