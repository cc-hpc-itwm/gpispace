#include "test_DaemonFSM_BSC.hpp"
#include <iostream>
#include <string>
#include <list>

#include <sdpa/events/RunJobEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>


using namespace std;
using namespace sdpa::tests;
using namespace sdpa::events;
using namespace sdpa::daemon;

CPPUNIT_TEST_SUITE_REGISTRATION( DaemonFSMTest_BSC );

DaemonFSMTest_BSC::DaemonFSMTest_BSC() : SDPA_INIT_LOGGER("sdpa.tests.DaemonFSMTest_BSC"),  m_DaemonFSM("orchestrator","'Output Stage'")
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

	string strFrom("");
	string strTo("");

	listEvents.push_back( new StartUpEvent(strFrom, strTo));
	listEvents.push_back( new ConfigOkEvent(strFrom, strTo));
	listEvents.push_back( new LifeSignEvent(strFrom, strTo));
	listEvents.push_back( new RequestJobEvent(strFrom, strTo));
	listEvents.push_back( new SubmitJobEvent(strFrom, strTo));
	while( !listEvents.empty() )
	{
		sc::event_base* pEvt = listEvents.front();
		m_DaemonFSM.process_event(*pEvt);

		listEvents.pop_front();
		delete dynamic_cast<MgmtEvent*>(pEvt);
	}

	std::vector<sdpa::job_id_t> vectorJobIDs = m_DaemonFSM.job_man_.getJobIDList();

	sdpa::job_id_t job_id = vectorJobIDs[0];
	RunJobEvent evtRun(strFrom, strTo, job_id);
	m_DaemonFSM.job_man_.job_map_[job_id]->RunJob(evtRun);

	JobFinishedEvent evtFinished(strFrom, strTo, job_id);
	m_DaemonFSM.job_man_.job_map_[job_id]->JobFinished(evtFinished);

	// now I#m in a final state and the delete must succeed
	DeleteJobEvent evtDelJob( strFrom, strTo, job_id );

	listEvents.push_back( new DeleteJobEvent(strFrom, strTo, job_id));
	listEvents.push_back( new ConfigRequestEvent(strFrom, strTo));
	listEvents.push_back( new InterruptEvent(strFrom, strTo));

	while( !listEvents.empty() )
	{
		sc::event_base* pEvt = listEvents.front();
		m_DaemonFSM.process_event(*pEvt);

		listEvents.pop_front();
		delete dynamic_cast<MgmtEvent*>(pEvt);
	}
}
