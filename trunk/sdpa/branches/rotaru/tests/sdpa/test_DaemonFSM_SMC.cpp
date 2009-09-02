#include "test_DaemonFSM_SMC.hpp"
#include <iostream>
#include <string>
#include <list>
#include <sdpa/memory.hpp>
#include <time.h>
#include <sdpa/util.hpp>
#include <fstream>

#include <sdpa/events/RunJobEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include "test_Sdpa2Gwes.hpp"

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::events;
using namespace sdpa::daemon;

CPPUNIT_TEST_SUITE_REGISTRATION( DaemonFSMTest_SMC );

DaemonFSMTest_SMC::DaemonFSMTest_SMC() : SDPA_INIT_LOGGER("sdpa.tests.DaemonFSMTest_SMC"),
  m_DaemonFSM("orchestrator","'Output Stage'", new DummyGwes )
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

	string strFromUp("user");
	string strFromDown("aggregator");
	string strTo = m_DaemonFSM.name();
	string strFrom = strTo;

    sdpa::util::time_type start(sdpa::util::now());

	StartUpEvent evtStartUp(strFrom, strTo);
	m_DaemonFSM.GetContext().StartUp(evtStartUp);

	ConfigOkEvent evtConfigOk(strFrom, strTo);
	m_DaemonFSM.GetContext().ConfigOk(evtConfigOk);

	Worker::ptr_t pWorker(new Worker(strFromDown));
	m_DaemonFSM.ptr_worker_man_->worker_map_[strFromDown] = pWorker;

	LifeSignEvent evtLS(strFromDown, strTo);
	m_DaemonFSM.GetContext().LifeSign(evtLS);

	RequestJobEvent evtReq(strFromDown, strTo);
	m_DaemonFSM.GetContext().RequestJob(evtReq);

	SubmitJobEvent evtSubmitJob(strFromUp, strTo);
	m_DaemonFSM.GetContext().SubmitJob(evtSubmitJob);

	std::vector<sdpa::job_id_t> vectorJobIDs = m_DaemonFSM.ptr_job_man_->getJobIDList();

	//Attention: delete succeeds only when the job should is in a final state!
	sdpa::job_id_t job_id = vectorJobIDs[0];
	RunJobEvent evtRun(strFrom, strTo, job_id);
	m_DaemonFSM.ptr_job_man_->job_map_[job_id]->RunJob(evtRun);

	JobFinishedEvent evtFinished(strFrom, strTo, job_id);
	m_DaemonFSM.ptr_job_man_->job_map_[job_id]->JobFinished(evtFinished);

	// now I#m in a final state and the delete must succeed
	DeleteJobEvent evtDelJob( strFromUp, strTo, vectorJobIDs[0] );
	m_DaemonFSM.GetContext().DeleteJob(evtDelJob);

	ConfigRequestEvent evtCfgReq(strFromDown, strTo);
	m_DaemonFSM.GetContext().ConfigRequest(evtCfgReq);

	InterruptEvent evtInt(strFrom, strTo);
	m_DaemonFSM.GetContext().Interrupt(evtInt);

	sdpa::util::time_type delta(sdpa::util::time_diff(start, sdpa::util::now()));
	std::cout << "smc: " << delta << "us" << std::endl;
}
