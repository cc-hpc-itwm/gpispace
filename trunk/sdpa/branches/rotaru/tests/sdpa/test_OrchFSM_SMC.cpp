#include "test_OrchFSM_SMC.hpp"
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

CPPUNIT_TEST_SUITE_REGISTRATION( OrchFSMTest_SMC );

OrchFSMTest_SMC::OrchFSMTest_SMC()
    : SDPA_INIT_LOGGER("sdpa.tests.OrchFSMTest_SMC")
{}

OrchFSMTest_SMC::~OrchFSMTest_SMC()
{}

void OrchFSMTest_SMC::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");
}

void OrchFSMTest_SMC::tearDown() { //stop the finite state machine
	SDPA_LOG_DEBUG("tearDown");
}

void OrchFSMTest_SMC::testOrchFSM_SMC()
{
	list<sdpa::shared_ptr<sc::event_base> > listEvents;
	string strEmpty("");

    sdpa::util::time_type start(sdpa::util::now());

	StartUpEvent evtStartUp(strEmpty,strEmpty);
	m_OrchFSM.GetContext().StartUp(evtStartUp);

	LifeSignEvent evtLS(strEmpty,strEmpty);
	m_OrchFSM.GetContext().LifeSign(evtLS);

	RequestJobEvent evtReq(strEmpty,strEmpty);
	m_OrchFSM.GetContext().RequestJob(evtReq);

	SubmitJobAckEvent evtSubmitJobAck(strEmpty,strEmpty);
	m_OrchFSM.GetContext().SubmitJobAck(evtSubmitJobAck);

	DeleteJobEvent evtDelJob(strEmpty,strEmpty);
	m_OrchFSM.GetContext().DeleteJob(evtDelJob);

	ConfigRequestEvent evtCfgReq(strEmpty,strEmpty);
	m_OrchFSM.GetContext().ConfigRequest(evtCfgReq);

	InterruptEvent evtInt(strEmpty,strEmpty);
	m_OrchFSM.GetContext().Interrupt(evtInt);

	sdpa::util::time_type delta(sdpa::util::time_diff(start, sdpa::util::now()));
	std::cout << "smc: " << delta << "us" << std::endl;
}
