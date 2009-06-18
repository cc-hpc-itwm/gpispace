#include "test_OrchFSM.hpp"
#include <iostream>
#include "sdpa/orchFSM/OrchFSM.hpp"
#include <string>
#include <list>

using namespace std;

using namespace sdpa::tests;
namespace sc = boost::statechart;

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
	list<sc::event_base*> listEvents;

	listEvents.push_back( new StartUpEvent());
	listEvents.push_back( new LifeSignEvent());

	listEvents.push_back( new RequestJobEvent());
	listEvents.push_back( new SubmitAckEvent());
	listEvents.push_back( new DeleteJobEvent());
	listEvents.push_back( new ConfigRequestEvent());
	listEvents.push_back( new InterruptEvent());


	while( !listEvents.empty() )
	{
		sc::event_base* pEvt = listEvents.front();
		m_OrchFSM.process_event(*pEvt);

		listEvents.pop_front();
		delete dynamic_cast<MgmtEvent*>(pEvt);
	}

}
