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
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>

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
			 SDPA_LOG_DEBUG("Received  WorkerRegistrationAckEvent!");
		 }
		 else if( dynamic_cast<SubmitJobAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received  SubmitJobAckEvent!");
		 }
		 else if( dynamic_cast<SubmitJobEvent*>(pEvt.get())  )
		 {
		 	 SDPA_LOG_DEBUG("Received  SubmitJobEvent!");
		 }
		 else if( dynamic_cast<JobFinishedAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received  JobFinishedAckEvent!");
		 }
		 else if( dynamic_cast<JobFailedAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received  JobFailedAckEvent!");
		 }
		 else if( dynamic_cast<JobStatusReplyEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received  JobStatusReplyEvent!");
		 }
		 else if( dynamic_cast<JobResultsReplyEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received  JobResultsReplyEvent!");
		 }
		 else if( dynamic_cast<JobFinishedEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received  JobFinishedEvent!");
		 }
		 else if( dynamic_cast<DeleteJobAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received  DeleteJobAckEvent!");
		 }
		 else if( dynamic_cast<JobResultsReplyEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received  JobResultsReplyEvent!");
		 }
		 else
		 {
		     SDPA_LOG_DEBUG("Received unexpected event!");
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

	 template <typename T>
	 sdpa::job_id_t WaitForEvent( )
	 {
		 seda::IEvent::Ptr pEvent;
		 ostringstream os;
		 std::string strName("");
		 os<<"Waiting for event "<<typeid(T).name()<<" ... ";
		 SDPA_LOG_DEBUG(os.str());

		 do
		 {
			pEvent = eventQueue.pop_and_wait();
			os.str("");
			os<<"Slave: Popped-up event "<<typeid(*(pEvent.get())).name();
			SDPA_LOG_DEBUG(os.str());
		 }while( typeid(*(pEvent.get())) != typeid(T) );

		 JobEvent* ptr = dynamic_cast<JobEvent*>(pEvent.get());

		 if(ptr)
			 return ptr->job_id();
		 else
			 return Job::invalid_job_id();
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
	m_ptrTestStrategy = seda::Strategy::Ptr( new TestStrategy("test") );
	//seda::AccumulateStrategy::Ptr ptrAccStrategy( new seda::AccumulateStrategy(ptrTestStrategy) );
	//m_ptrOutputStage = shared_ptr<seda::Stage>( new seda::Stage("output_stage", ptrAccStrategy) );

	m_ptrOutputStage = shared_ptr<seda::Stage>( new seda::Stage("output_stage", m_ptrTestStrategy) );
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

void DaemonFSMTest_SMC::testDaemonFSM_SMC_JobFinished()
{
	list<sdpa::shared_ptr<sc::event_base> > listEvents;

	string strFromUp("user");
	string strFromDown("aggregator");
	string strDaemon   = m_ptrDaemonFSM->name();
	//ring strDaemon = strDaemon;

	ostringstream os;
	TestStrategy* pTestStr = dynamic_cast<TestStrategy*>(m_ptrTestStrategy.get());

    sdpa::util::time_type start(sdpa::util::now());
    StartUpEvent::Ptr pEvtStartUp(new StartUpEvent(strDaemon, strDaemon));
	m_ptrDaemonFSM->daemon_stage_->send(pEvtStartUp);//GetContext().StartUp(evtStartUp);

	ConfigOkEvent::Ptr pEvtConfigOk( new ConfigOkEvent(strDaemon, strDaemon));
	m_ptrDaemonFSM->daemon_stage_->send(pEvtConfigOk); //GetContext().ConfigOk(evtConfigOk);

	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage_->send(pEvtWorkerReg); //GetContext().RegisterWorker(evtWorkerReg);

	/*nfigRequestEvent evtCfgReq(strFromDown, strDaemon);
	m_ptrDaemonFSM->GetContext().ConfigRequest(evtCfgReq);

	InterruptEvent evtInt(strDaemon, strDaemon);
	m_ptrDaemonFSM->GetContext().Interrupt(evtInt);*/

    for(int k=0;k<1; k++)
    {
    	cout<<"************ITERATION "<<k<<" ************************"<<std::endl;

		LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromDown, strDaemon));
		m_ptrDaemonFSM->daemon_stage_->send(pEvtLS);

		// the user submits a job
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUp, strDaemon));
		m_ptrDaemonFSM->daemon_stage_->send(pEvtSubmitJob);

		sdpa::job_id_t job_id_user = pTestStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>();

		sleep(1); // give time to GWES to produce new jobs

		// the slave posts a job request
		RequestJobEvent::Ptr pEvtReq( new RequestJobEvent(strFromDown, strDaemon) );
		m_ptrDaemonFSM->daemon_stage_->send(pEvtReq);

		// wait the master to reply to the job request
		sdpa::job_id_t job_id_slave = pTestStr->WaitForEvent<sdpa::events::SubmitJobEvent>();

		// send a SubmitJobAckEvent to master
		// the master should acknowledge the job then
		SubmitJobAckEvent::Ptr pSubmitJobAck( new SubmitJobAckEvent(strFromDown, strDaemon, job_id_slave) );
		m_ptrDaemonFSM->daemon_stage_->send(pSubmitJobAck);

		// the slave computes the job

		// submit a JobFinishedEvent to master
		JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(strFromDown, strDaemon, job_id_slave));
		m_ptrDaemonFSM->daemon_stage_->send(pEvtJobFinished);

		pTestStr->WaitForEvent<sdpa::events::JobFinishedAckEvent>();

		// check if the job finished
		QueryJobStatusEvent::Ptr pEvtQueryStatus(new QueryJobStatusEvent(strFromUp, strDaemon, job_id_user));
		m_ptrDaemonFSM->daemon_stage_->send(pEvtQueryStatus);
		// wait for a JobStatusReplyEvent
		pTestStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>();

		// if the job is in the finished or failed state, one is allowed
		// to retriieve the results now
		RetrieveJobResultsEvent::Ptr pEvtRetrieveRes(new RetrieveJobResultsEvent(strFromUp, strDaemon, job_id_user));
		m_ptrDaemonFSM->daemon_stage_->send(pEvtRetrieveRes);
		// wait for a JobStatusReplyEvent
		pTestStr->WaitForEvent<sdpa::events::JobResultsReplyEvent>();

		// check the job status. if the job is in a final state, send a DeletJobEvent
		DeleteJobEvent::Ptr pEvtDelJob( new DeleteJobEvent(strFromUp, strDaemon, job_id_user) );
		m_ptrDaemonFSM->daemon_stage_->send(pEvtDelJob);

		// wait for an acknowledgment from Orchestrator that job was deleted
		sdpa::job_id_t jobid = pTestStr->WaitForEvent<sdpa::events::DeleteJobAckEvent>();
		os.str("");
		os<<"Successfully deleted the job "<<jobid;
		SDPA_LOG_DEBUG(os.str());

		// you can leave now
		SDPA_LOG_DEBUG("Slave: Finished!");
    }

	// request the results before deleting the job!
}
