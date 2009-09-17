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

#include <seda/Stage.hpp>
#include <seda/Strategy.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/AccumulateStrategy.hpp>


using namespace std;
using namespace sdpa::tests;
using namespace sdpa::events;
using namespace sdpa::daemon;

class TestStrategy : public seda::Strategy
{
public:
	 typedef std::tr1::shared_ptr<TestStrategy> Ptr;
	 TestStrategy(const std::string& name): seda::Strategy(name), SDPA_INIT_LOGGER("TestStrategy")  {}
	 void perform(const seda::IEvent::Ptr& pEvt) {

		 if( dynamic_cast<WorkerRegistrationAckEvent*>(pEvt.get())  )
			 SDPA_LOG_DEBUG("Received WorkerRegistrationAckEvent!");
		 else if( dynamic_cast<SubmitJobAckEvent*>(pEvt.get())  )
			 SDPA_LOG_DEBUG("Received SubmitJobAckEvent!");
		 else if( dynamic_cast<SubmitJobEvent*>(pEvt.get())  )
		 	 SDPA_LOG_DEBUG("Received SubmitJobEvent!");
		 else
		     SDPA_LOG_DEBUG("Unexpected event!");
	 }

	 SDPA_DECLARE_LOGGER();
};

CPPUNIT_TEST_SUITE_REGISTRATION( DaemonFSMTest_SMC );

DaemonFSMTest_SMC::DaemonFSMTest_SMC() : m_ptrSdpa2Gwes(new DummyGwes),
	SDPA_INIT_LOGGER("sdpa.tests.DaemonFSMTest_SMC")
{
	seda::Strategy::Ptr ptrTestStrategy( new TestStrategy("test") );
	seda::AccumulateStrategy::Ptr ptrAccStrategy( new seda::AccumulateStrategy(ptrTestStrategy) );

	m_ptrOutputStage = shared_ptr<seda::Stage>( new seda::Stage("output_stage", ptrAccStrategy) );
	m_ptrDaemonFSM = shared_ptr<sdpa::fsm::smc::DaemonFSM>(new sdpa::fsm::smc::DaemonFSM("orchestrator","output_stage", m_ptrSdpa2Gwes.get()));
}

DaemonFSMTest_SMC::~DaemonFSMTest_SMC()
{}

void DaemonFSMTest_SMC::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	sdpa::fsm::smc::DaemonFSM::start(m_ptrDaemonFSM);

	//create output stage
	seda::StageRegistry::instance().insert(m_ptrOutputStage);
	m_ptrOutputStage->start();
}

void DaemonFSMTest_SMC::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	seda::StageRegistry::instance().lookup("orchestrator")->stop();
	seda::StageRegistry::instance().lookup("output_stage")->stop();
	seda::StageRegistry::instance().clear();
}

void DaemonFSMTest_SMC::testDaemonFSM_SMC()
{
	list<sdpa::shared_ptr<sc::event_base> > listEvents;

	string strFromUp("user");
	string strFromDown("aggregator");
	string strTo   = m_ptrDaemonFSM->name();
	string strFrom = strTo;

    sdpa::util::time_type start(sdpa::util::now());

	StartUpEvent::Ptr pEvtStartUp(new StartUpEvent(strFrom, strTo));
	m_ptrDaemonFSM->daemon_stage_->send(pEvtStartUp);//GetContext().StartUp(evtStartUp);

	ConfigOkEvent::Ptr pEvtConfigOk( new ConfigOkEvent(strFrom, strTo));
	m_ptrDaemonFSM->daemon_stage_->send(pEvtConfigOk); //GetContext().ConfigOk(evtConfigOk);

	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromDown, strTo));
	m_ptrDaemonFSM->daemon_stage_->send(pEvtWorkerReg); //GetContext().RegisterWorker(evtWorkerReg);

	LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromDown, strTo));
	m_ptrDaemonFSM->daemon_stage_->send(pEvtLS); //GetContext().LifeSign(evtLS);*/

	// the user submits a job
	SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUp, strTo));
	m_ptrDaemonFSM->daemon_stage_->send(pEvtSubmitJob); //GetContext().SubmitJob(evtSubmitJob1);

	sleep(1);
	// post a job request
	RequestJobEvent::Ptr pEvtReq( new RequestJobEvent(strFromDown, strTo) );
	m_ptrDaemonFSM->daemon_stage_->send(pEvtReq);

	seda::AccumulateStrategy* pAcc = dynamic_cast<seda::AccumulateStrategy*>(m_ptrOutputStage->strategy().get());
	ostringstream os;
    os<<"Sequence of events sent to the output stage: "<<std::endl<<pAcc->str();
    SDPA_LOG_DEBUG(os.str());


	SDPA_LOG_DEBUG("Finished!");
	// request the results before deleting the job!

	/*
	// now I#m in a final state and the delete must succeed
	std::vector<sdpa::job_id_t> vectorJobIDs = m_ptrDaemonFSM->ptr_job_man_->getJobIDList();

	//Attention: delete succeeds only when the job should is in a final state!
	sdpa::job_id_t job_id = vectorJobIDs[0];

	JobFinishedEvent evtFinished(strFrom, strTo, job_id);
	m_ptrDaemonFSM->ptr_job_man_->job_map_[job_id]->process_event(evtFinished);


	 DeleteJobEvent evtDelJob( strFromUp, strTo, vectorJobIDs[0] );
	m_ptrDaemonFSM->GetContext().DeleteJob(evtDelJob);

	ConfigRequestEvent evtCfgReq(strFromDown, strTo);
	m_ptrDaemonFSM->GetContext().ConfigRequest(evtCfgReq);

	InterruptEvent evtInt(strFrom, strTo);
	m_ptrDaemonFSM->GetContext().Interrupt(evtInt);*/
}
