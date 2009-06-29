#include "test_DaemonFSM_SMC.hpp"
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

CPPUNIT_TEST_SUITE_REGISTRATION( DaemonFSMTest_SMC );

DaemonFSMTest_SMC::DaemonFSMTest_SMC() : SDPA_INIT_LOGGER("sdpa.tests.DaemonFSMTest_SMC"), m_DaemonFSM("","")
{}

DaemonFSMTest_SMC::~DaemonFSMTest_SMC()
{}

void DaemonFSMTest_SMC::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");
}

void DaemonFSMTest_SMC::tearDown() { //stop the finite state machine
	SDPA_LOG_DEBUG("tearDown");
}

void DaemonFSMTest_SMC::testDaemonFSM_SMC()
{
	list<sdpa::shared_ptr<sc::event_base> > listEvents;
	string strEmpty("");

    sdpa::util::time_type start(sdpa::util::now());

	StartUpEvent evtStartUp(strEmpty,strEmpty);
	m_DaemonFSM.GetContext().StartUp(evtStartUp);

	LifeSignEvent evtLS(strEmpty,strEmpty);
	m_DaemonFSM.GetContext().LifeSign(evtLS);

	RequestJobEvent evtReq(strEmpty,strEmpty);
	m_DaemonFSM.GetContext().RequestJob(evtReq);

	SubmitJobAckEvent evtSubmitJobAck(strEmpty,strEmpty);
	m_DaemonFSM.GetContext().SubmitJobAck(evtSubmitJobAck);

	DeleteJobEvent evtDelJob(strEmpty,strEmpty);
	m_DaemonFSM.GetContext().DeleteJob(evtDelJob);

	ConfigRequestEvent evtCfgReq(strEmpty,strEmpty);
	m_DaemonFSM.GetContext().ConfigRequest(evtCfgReq);

	InterruptEvent evtInt(strEmpty,strEmpty);
	m_DaemonFSM.GetContext().Interrupt(evtInt);

	sdpa::util::time_type delta(sdpa::util::time_diff(start, sdpa::util::now()));
	std::cout << "smc: " << delta << "us" << std::endl;
}
