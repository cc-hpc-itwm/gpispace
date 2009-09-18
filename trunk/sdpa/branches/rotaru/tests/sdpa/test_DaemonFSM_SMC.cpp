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
#include <seda/StageRegistry.hpp>


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
		 {
			 SDPA_LOG_DEBUG("Pushed WorkerRegistrationAckEvent!");
		 }
		 else if( dynamic_cast<SubmitJobAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Pushed SubmitJobAckEvent!");
		 }
		 else if( dynamic_cast<SubmitJobEvent*>(pEvt.get())  )
		 {
		 	 SDPA_LOG_DEBUG("Pushed SubmitJobEvent!");
		 }
		 else
		 {
		     SDPA_LOG_DEBUG("Unexpected event!");
		 }

		 eventQueue.push(pEvt);
	 }

	 std::string str()
	 {
		std::ostringstream ostream(std::ostringstream::out);
		eventQueue_t::iterator it;
		for (it = eventQueue.begin(); it != eventQueue.end(); it++) {
			ostream <<typeid(*(it->get())).name() << std::endl;
		}
		return ostream.str();
	 }

	 typedef SynchronizedQueue<std::list<seda::IEvent::Ptr> > eventQueue_t;

	 eventQueue_t eventQueue;

	 SDPA_DECLARE_LOGGER();
};

CPPUNIT_TEST_SUITE_REGISTRATION( DaemonFSMTest_SMC );

DaemonFSMTest_SMC::DaemonFSMTest_SMC() :
	SDPA_INIT_LOGGER("sdpa.tests.DaemonFSMTest_SMC")
{
}

DaemonFSMTest_SMC::~DaemonFSMTest_SMC()
{}

void DaemonFSMTest_SMC::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	m_ptrSdpa2Gwes = sdpa::wf::Sdpa2Gwes::ptr_t (new DummyGwes);
	ptrTestStrategy = seda::Strategy::Ptr( new TestStrategy("test") );
	//seda::AccumulateStrategy::Ptr ptrAccStrategy( new seda::AccumulateStrategy(ptrTestStrategy) );
	//m_ptrOutputStage = shared_ptr<seda::Stage>( new seda::Stage("output_stage", ptrAccStrategy) );

	m_ptrOutputStage = shared_ptr<seda::Stage>( new seda::Stage("output_stage", ptrTestStrategy) );
	m_ptrDaemonFSM = shared_ptr<sdpa::fsm::smc::DaemonFSM>(new sdpa::fsm::smc::DaemonFSM("orchestrator","output_stage", m_ptrSdpa2Gwes.get()));

	sdpa::fsm::smc::DaemonFSM::start(m_ptrDaemonFSM);

	//create output stage
	seda::StageRegistry::instance().insert(m_ptrOutputStage);
	m_ptrOutputStage->start();
}

void DaemonFSMTest_SMC::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	m_ptrSdpa2Gwes.reset();

	seda::StageRegistry::instance().lookup("orchestrator")->stop();
	seda::StageRegistry::instance().lookup("output_stage")->stop();
	seda::StageRegistry::instance().clear();

	m_ptrOutputStage.reset();
	m_ptrDaemonFSM.reset();
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

	ostringstream os;
	TestStrategy* pTestStr = dynamic_cast<TestStrategy*>(ptrTestStrategy.get());
	os<<"Sequence of events sent to the output stage: "<<std::endl<<pTestStr->str();
	SDPA_LOG_DEBUG(os.str());

	seda::IEvent::Ptr pEvent;
	// user waits for acknowledgement
	do
	{
		pEvent = pTestStr->eventQueue.pop_and_wait();
		os.str("");
		os<<"Popped-up event "<<typeid(*(pEvent.get())).name();
		SDPA_LOG_DEBUG(os.str());
	}while(typeid(*(pEvent.get())) != typeid(sdpa::events::SubmitJobAckEvent));

	sleep(1); //leave time for GWES to produce new jobs

	// slave post a job request
	RequestJobEvent::Ptr pEvtReq( new RequestJobEvent(strFromDown, strTo) );
	m_ptrDaemonFSM->daemon_stage_->send(pEvtReq);

	//wait until the request is served
	do
	{
		pEvent = pTestStr->eventQueue.pop_and_wait();
		os.str("");
		os<<"Popped-up event "<<typeid(*(pEvent.get())).name();
		SDPA_LOG_DEBUG(os.str());
	}while(typeid(*(pEvent.get())) != typeid(sdpa::events::SubmitJobEvent));

	// submit a JobFinishedEvent to Orchestrator/Aggregator
	// the user submits a job
	sdpa::job_id_t job_id = dynamic_cast<sdpa::events::JobEvent*>(pEvent.get())->job_id();
	JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(strFromDown, strTo, job_id));
	m_ptrDaemonFSM->daemon_stage_->send(pEvtJobFinished); //GetContext().SubmitJob(evtSubmitJob1);


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
