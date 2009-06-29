#include "test_DaemonFSM_BSC.hpp"
#include <iostream>
#include <string>
#include <list>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::events;

CPPUNIT_TEST_SUITE_REGISTRATION( DaemonFSMTest_BSC );

DaemonFSMTest_BSC::DaemonFSMTest_BSC() : SDPA_INIT_LOGGER("sdpa.tests.DaemonFSMTest_BSC"), m_DaemonFSM("","")
{}

DaemonFSMTest_BSC::~DaemonFSMTest_BSC()
{}

void DaemonFSMTest_BSC::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");
	m_DaemonFSM.initiate();
}

void DaemonFSMTest_BSC::tearDown() { //stop the finite state machine
	SDPA_LOG_DEBUG("tearDown");
}

void DaemonFSMTest_BSC::testDaemonFSM_BSC()
{
	list<sc::event_base*> listEvents;

	string strEmpty("");

	listEvents.push_back( new StartUpEvent(strEmpty,strEmpty));
	listEvents.push_back( new LifeSignEvent(strEmpty,strEmpty));
	listEvents.push_back( new RequestJobEvent(strEmpty,strEmpty));
	listEvents.push_back( new SubmitJobAckEvent(strEmpty,strEmpty));
	listEvents.push_back( new DeleteJobEvent(strEmpty,strEmpty));
	listEvents.push_back( new ConfigRequestEvent(strEmpty,strEmpty));
	listEvents.push_back( new InterruptEvent(strEmpty,strEmpty));

	while( !listEvents.empty() )
	{
		sc::event_base* pEvt = listEvents.front();
		m_DaemonFSM.process_event(*pEvt);

		listEvents.pop_front();
		delete dynamic_cast<MgmtEvent*>(pEvt);
	}
}
