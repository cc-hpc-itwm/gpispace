#include "test_DaemonFSM_SMC.hpp"
#include <iostream>
#include <string>
#include <list>
#include <sdpa/memory.hpp>
#include <time.h>
#include <sdpa/util.hpp>
#include <fstream>

#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>
#include "DummyGwes.hpp"


#include <seda/StageRegistry.hpp>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::events;
using namespace sdpa::daemon;

CPPUNIT_TEST_SUITE_REGISTRATION( DaemonFSMTest_SMC );

DaemonFSMTest_SMC::DaemonFSMTest_SMC() :
	m_ptrDaemonFSM(new sdpa::fsm::smc::DaemonFSM("orchestrator","'Output Stage'", new DummyGwes)),
	SDPA_INIT_LOGGER("sdpa.tests.DaemonFSMTest_SMC")
{
}

DaemonFSMTest_SMC::~DaemonFSMTest_SMC()
{}

void DaemonFSMTest_SMC::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	sdpa::fsm::smc::DaemonFSM::start(m_ptrDaemonFSM);

}

void DaemonFSMTest_SMC::tearDown() { //stop the finite state machine

	seda::StageRegistry::instance().lookup("orchestrator")->stop();

	ostringstream os;

	SDPA_LOG_DEBUG("Reset the pointer to the daemon state machine");
	m_ptrDaemonFSM.reset();
	SDPA_LOG_DEBUG("tearDown");

	seda::StageRegistry::instance().clear();

}

void DaemonFSMTest_SMC::testDaemonFSM_SMC()
{
	list<sdpa::shared_ptr<sc::event_base> > listEvents;

	string strFromUp("user");
	string strFromDown("aggregator");
	string strTo = m_ptrDaemonFSM->name();
	string strFrom = strTo;

    sdpa::util::time_type start(sdpa::util::now());

	StartUpEvent evtStartUp(strFrom, strTo);
	m_ptrDaemonFSM->GetContext().StartUp(evtStartUp);

	ConfigOkEvent evtConfigOk(strFrom, strTo);
	m_ptrDaemonFSM->GetContext().ConfigOk(evtConfigOk);

	WorkerRegistrationEvent evtWorkerReg(strFromDown, strTo);
	m_ptrDaemonFSM->GetContext().RegisterWorker(evtWorkerReg);

	LifeSignEvent evtLS(strFromDown, strTo);
	m_ptrDaemonFSM->GetContext().LifeSign(evtLS);

	// send an external job
	SubmitJobEvent evtSubmitJob1(strFromUp, strTo);
	m_ptrDaemonFSM->GetContext().SubmitJob(evtSubmitJob1);

	// send a local job
	SubmitJobEvent evtSubmitJob2(strTo, strTo);
	m_ptrDaemonFSM->GetContext().SubmitJob(evtSubmitJob2);

	std::vector<sdpa::job_id_t> vectorJobIDs = m_ptrDaemonFSM->ptr_job_man_->getJobIDList();

	//Attention: delete succeeds only when the job should is in a final state!
	sdpa::job_id_t job_id = vectorJobIDs[0];
	SubmitJobEvent evtSubmit(strFrom, strTo, job_id);
	m_ptrDaemonFSM->ptr_job_man_->job_map_[job_id]->process_event(evtSubmit);

	JobFinishedEvent evtFinished(strFrom, strTo, job_id);
	m_ptrDaemonFSM->ptr_job_man_->job_map_[job_id]->process_event(evtFinished);

	// post a job request
	RequestJobEvent evtReq(strFromDown, strTo);
	m_ptrDaemonFSM->GetContext().RequestJob(evtReq);

	// now I#m in a final state and the delete must succeed
	DeleteJobEvent evtDelJob( strFromUp, strTo, vectorJobIDs[0] );
	m_ptrDaemonFSM->GetContext().DeleteJob(evtDelJob);

	ConfigRequestEvent evtCfgReq(strFromDown, strTo);
	m_ptrDaemonFSM->GetContext().ConfigRequest(evtCfgReq);

	InterruptEvent evtInt(strFrom, strTo);
	m_ptrDaemonFSM->GetContext().Interrupt(evtInt);
}
