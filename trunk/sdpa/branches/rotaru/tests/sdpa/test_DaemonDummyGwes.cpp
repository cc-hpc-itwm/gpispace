#include "test_DaemonDummyGwes.hpp"

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
#include <sdpa/events/WorkerRegistrationAckEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/ConfigRequestEvent.hpp>
#include <sdpa/events/ConfigReplyEvent.hpp>

#include <boost/shared_ptr.hpp>
#include "DummyGwes.hpp"

#include <seda/Stage.hpp>
#include <seda/StageRegistry.hpp>


using namespace std;
using namespace sdpa::tests;
using namespace sdpa::events;
using namespace sdpa::daemon;

const int NITER = 1000;

class TestStrategy : public seda::Strategy
{
public:
	 typedef std::tr1::shared_ptr<TestStrategy> Ptr;
	 TestStrategy(const std::string& name): seda::Strategy(name), SDPA_INIT_LOGGER("TestStrategy")  {}
	 void perform(const seda::IEvent::Ptr& pEvt) {

		 if( dynamic_cast<WorkerRegistrationAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received ConfigRequestEvent!");
		 }
		 else if( dynamic_cast<WorkerRegistrationAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received ConfigReplyEvent!");
		 }
		 if( dynamic_cast<WorkerRegistrationAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received WorkerRegistrationAckEvent!");
		 }
		 else if( dynamic_cast<SubmitJobAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received SubmitJobAckEvent!");
		 }
		 else if( dynamic_cast<SubmitJobEvent*>(pEvt.get())  )
		 {
		 	 SDPA_LOG_DEBUG("Received SubmitJobEvent!");
		 }
		 else if( dynamic_cast<JobFinishedAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received JobFinishedAckEvent!");
		 }
		 else if( dynamic_cast<JobFailedAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received JobFailedAckEvent!");
		 }
		 else if( dynamic_cast<JobStatusReplyEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received JobStatusReplyEvent!");
		 }
		 else if( dynamic_cast<JobResultsReplyEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received JobResultsReplyEvent!");
		 }
		 else if( dynamic_cast<JobFinishedEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received JobFinishedEvent!");
		 }
		 else if( dynamic_cast<DeleteJobAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received DeleteJobAckEvent!");
		 }
		 else if( dynamic_cast<JobResultsReplyEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received JobResultsReplyEvent!");
		 }
		 else if( dynamic_cast<CancelJobEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received CancelJobEvent!");
		 }
		 else if( dynamic_cast<CancelJobAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received CancelJobAckEvent!");
		 }
		 else if( dynamic_cast<ErrorEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received ErrorEvent!");
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
	 typename T::Ptr WaitForEvent(sdpa::events::ErrorEvent::Ptr& pErrorEvt )
	 {
		 seda::IEvent::Ptr pEvent;
		 pErrorEvt.reset();

		 ostringstream os;
		 std::string strName("");
		 os<<"Waiting for event "<<typeid(T).name()<<" ... ";
		 SDPA_LOG_DEBUG(os.str());

		 typename T::Ptr ptrT;
		 do
		 {
			pEvent = eventQueue.pop_and_wait();
			os.str("");
			os<<"Slave: Popped-up event "<<typeid(*(pEvent.get())).name();
			SDPA_LOG_DEBUG(os.str());

#if USE_STL_TR1
			ptrT = std::tr1::dynamic_pointer_cast<T, seda::IEvent>( pEvent );
			pErrorEvt = std::tr1::dynamic_pointer_cast<sdpa::events::ErrorEvent, seda::IEvent>( pEvent );
#else
			ptrT = boost::dynamic_pointer_cast<T, seda::IEvent>( pEvent );
			pErrorEvt = boost::dynamic_pointer_cast<sdpa::events::ErrorEvent, seda::IEvent>( pEvent );
#endif

		 }while( !ptrT.get() && !pErrorEvt.get());

		 return  ptrT;
	 }

	 typedef SynchronizedQueue<std::list<seda::IEvent::Ptr> > eventQueue_t;

	 eventQueue_t eventQueue;

	 SDPA_DECLARE_LOGGER();
};

CPPUNIT_TEST_SUITE_REGISTRATION( DaemonFSMTest );

DaemonFSMTest::DaemonFSMTest() :
	SDPA_INIT_LOGGER("sdpa.tests.DaemonFSMTest")
{
}

DaemonFSMTest::~DaemonFSMTest()
{}

void DaemonFSMTest::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	m_ptrSdpa2Gwes = new DummyGwes();
	m_ptrTestStrategy = seda::Strategy::Ptr( new TestStrategy("test") );

	m_ptrOutputStage = shared_ptr<seda::Stage>( new seda::Stage("output_stage", m_ptrTestStrategy) );
	m_ptrDaemonFSM = shared_ptr<DaemonFSM>(new DaemonFSM("orchestrator","output_stage", m_ptrSdpa2Gwes));

	DaemonFSM::start(m_ptrDaemonFSM);

	//create output stage
	seda::StageRegistry::instance().insert(m_ptrOutputStage);
	m_ptrOutputStage->start();
}

void DaemonFSMTest::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	seda::StageRegistry::instance().lookup(m_ptrDaemonFSM->name())->stop();
	seda::StageRegistry::instance().lookup(m_ptrOutputStage->name())->stop();
	seda::StageRegistry::instance().clear();

	m_ptrOutputStage.reset();
	m_ptrDaemonFSM.reset();
	delete m_ptrSdpa2Gwes;
}

void DaemonFSMTest::testDaemonFSM_JobFinished()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobFinished******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string strFromUp("user");
	string strFromDown("aggregator");
	string strDaemon   = m_ptrDaemonFSM->name();
	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	TestStrategy* pTestStr = dynamic_cast<TestStrategy*>(m_ptrTestStrategy.get());

    sdpa::util::time_type start(sdpa::util::now());
    //start-up the orchestrator
    StartUpEvent::Ptr pEvtStartUp(new StartUpEvent(strDaemon, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtStartUp);

	ConfigOkEvent::Ptr pEvtConfigOk( new ConfigOkEvent(strDaemon, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtConfigOk);

	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtWorkerReg);
	pTestStr->WaitForEvent<sdpa::events::WorkerRegistrationAckEvent>(pErrorEvt);

	ConfigRequestEvent::Ptr pEvtCfgReq( new ConfigRequestEvent(strFromDown, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtCfgReq);

	// wait for a configuration reply event
	pTestStr->WaitForEvent<sdpa::events::ConfigReplyEvent>(pErrorEvt);

	LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtLS);

	for(int k=0; k<NITER;k++) {
	// the user submits a job
	SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUp, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtSubmitJob);

	// the user waits for an acknowledgment
	sdpa::job_id_t job_id_user = pTestStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

	// the slave posts a job request
	RequestJobEvent::Ptr pEvtReq( new RequestJobEvent(strFromDown, strDaemon) );
	m_ptrDaemonFSM->daemon_stage()->send(pEvtReq);

	sdpa::job_id_t job_id_slave;
	SubmitJobEvent::Ptr pSubmitJobEvent = pTestStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
	while( pErrorEvt.get() )
	{
		if(pErrorEvt->error_code() == ErrorEvent::SDPA_ENOJOBAVAIL)
		{
			os.str("");
			os<<"No job available! Try again ...";
			SDPA_LOG_DEBUG(os.str());

			//Post new reqest and wait
			RequestJobEvent::Ptr pEvtReqNew( new RequestJobEvent(strFromDown, strDaemon) );
			m_ptrDaemonFSM->daemon_stage()->send(pEvtReqNew);
			// wait the master to reply to the job request
			pErrorEvt.reset();
			pSubmitJobEvent = pTestStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
		}
	}

	SDPA_LOG_DEBUG("Try to set the JobId ...");
	job_id_slave = pSubmitJobEvent->job_id();

	// send a SubmitJobAckEvent to master
	// the master should acknowledge the job then
	SubmitJobAckEvent::Ptr pSubmitJobAck( new SubmitJobAckEvent(strFromDown, strDaemon, job_id_slave) );
	m_ptrDaemonFSM->daemon_stage()->send(pSubmitJobAck);

	// the slave computes the job ........

	// submit a JobFinishedEvent to master
	JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(strFromDown, strDaemon, job_id_slave));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtJobFinished);

	pTestStr->WaitForEvent<sdpa::events::JobFinishedAckEvent>(pErrorEvt);

	// check if the job finished
	QueryJobStatusEvent::Ptr pEvtQueryStatus(new QueryJobStatusEvent(strFromUp, strDaemon, job_id_user));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtQueryStatus);

	// wait for a JobStatusReplyEvent
	JobStatusReplyEvent::Ptr pJobStatusReplyEvent = pTestStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
	os.str("");
	os<<"The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status();
	SDPA_LOG_DEBUG(os.str());

	if( pJobStatusReplyEvent->status().find("Finished") != std::string::npos ||
		pJobStatusReplyEvent->status().find("Failed")   != std::string::npos )
	{
		// if the job is in the finished or failed state, one is allowed
		// to retriieve the results now
		RetrieveJobResultsEvent::Ptr pEvtRetrieveRes(new RetrieveJobResultsEvent(strFromUp, strDaemon, job_id_user));
		m_ptrDaemonFSM->daemon_stage()->send(pEvtRetrieveRes);
		// wait for a JobStatusReplyEvent
		pTestStr->WaitForEvent<sdpa::events::JobResultsReplyEvent>(pErrorEvt);

		// check the job status. if the job is in a final state, send a DeletJobEvent
		DeleteJobEvent::Ptr pEvtDelJob( new DeleteJobEvent(strFromUp, strDaemon, job_id_user) );
		m_ptrDaemonFSM->daemon_stage()->send(pEvtDelJob);

		// wait for an acknowledgment from Orchestrator that job was deleted
		sdpa::job_id_t jobid = pTestStr->WaitForEvent<sdpa::events::DeleteJobAckEvent>(pErrorEvt)->job_id();
		os.str("");
		os<<"Successfully deleted the job "<<jobid;
		SDPA_LOG_DEBUG(os.str());
	}
	else
		SDPA_LOG_ERROR("The job is supposed to be into a 'terminal state' in order to be able to retrieve results!");

	}

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtInt( new InterruptEvent(strDaemon, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtInt);

	// you can leave now
	SDPA_LOG_DEBUG("Slave: Finished!");
}


void DaemonFSMTest::testDaemonFSM_JobFailed()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobFailed******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string strFromUp("user");
	string strFromDown("aggregator");
	string strDaemon   = m_ptrDaemonFSM->name();
	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	TestStrategy* pTestStr = dynamic_cast<TestStrategy*>(m_ptrTestStrategy.get());

    sdpa::util::time_type start(sdpa::util::now());
    //start-up the orchestrator
    StartUpEvent::Ptr pEvtStartUp(new StartUpEvent(strDaemon, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtStartUp);

	ConfigOkEvent::Ptr pEvtConfigOk( new ConfigOkEvent(strDaemon, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtConfigOk);

	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtWorkerReg);
	pTestStr->WaitForEvent<sdpa::events::WorkerRegistrationAckEvent>(pErrorEvt);

	ConfigRequestEvent::Ptr pEvtCfgReq( new ConfigRequestEvent(strFromDown, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtCfgReq);

	// wait for a configuration reply event
	pTestStr->WaitForEvent<sdpa::events::ConfigReplyEvent>(pErrorEvt);

	LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtLS);

	for(int k=0; k<NITER;k++) {
	// the user submits a job
	SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUp, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtSubmitJob);

	// the user waits for an acknowledgment
	sdpa::job_id_t job_id_user = pTestStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

	// the slave posts a job request
	RequestJobEvent::Ptr pEvtReq( new RequestJobEvent(strFromDown, strDaemon) );
	m_ptrDaemonFSM->daemon_stage()->send(pEvtReq);

	sdpa::job_id_t job_id_slave;
	SubmitJobEvent::Ptr pSubmitJobEvent = pTestStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
	while( pErrorEvt.get() )
	{
		if(pErrorEvt->error_code() == ErrorEvent::SDPA_ENOJOBAVAIL)
		{
			os.str("");
			os<<"No job available! Try again ...";
			SDPA_LOG_DEBUG(os.str());

			//Post new reqest and wait
			RequestJobEvent::Ptr pEvtReqNew( new RequestJobEvent(strFromDown, strDaemon) );
			m_ptrDaemonFSM->daemon_stage()->send(pEvtReqNew);
			// wait the master to reply to the job request
			pErrorEvt.reset();
			pSubmitJobEvent = pTestStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
		}
	}

	SDPA_LOG_DEBUG("Try to set the JobId ...");
	job_id_slave = pSubmitJobEvent->job_id();

	// send a SubmitJobAckEvent to master
	// the master should acknowledge the job then
	SubmitJobAckEvent::Ptr pSubmitJobAck( new SubmitJobAckEvent(strFromDown, strDaemon, job_id_slave) );
	m_ptrDaemonFSM->daemon_stage()->send(pSubmitJobAck);

	// the slave computes the job and it fails .....

	// submit a JobFinishedEvent to master
	JobFailedEvent::Ptr pEvtJobFailed(new JobFailedEvent(strFromDown, strDaemon, job_id_slave));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtJobFailed);

	pTestStr->WaitForEvent<sdpa::events::JobFailedAckEvent>(pErrorEvt);

	// check if the job finished
	QueryJobStatusEvent::Ptr pEvtQueryStatus(new QueryJobStatusEvent(strFromUp, strDaemon, job_id_user));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtQueryStatus);

	// wait for a JobStatusReplyEvent
	JobStatusReplyEvent::Ptr pJobStatusReplyEvent = pTestStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
	os.str("");
	os<<"The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status();
	SDPA_LOG_DEBUG(os.str());

	if( pJobStatusReplyEvent->status().find("Finished") != std::string::npos ||
		pJobStatusReplyEvent->status().find("Failed")   != std::string::npos )
	{
		// if the job is in the finished or failed state, one is allowed
		// to retriieve the results now
		RetrieveJobResultsEvent::Ptr pEvtRetrieveRes(new RetrieveJobResultsEvent(strFromUp, strDaemon, job_id_user));
		m_ptrDaemonFSM->daemon_stage()->send(pEvtRetrieveRes);
		// wait for a JobStatusReplyEvent
		pTestStr->WaitForEvent<sdpa::events::JobResultsReplyEvent>(pErrorEvt);

		// check the job status. if the job is in a final state, send a DeletJobEvent
		DeleteJobEvent::Ptr pEvtDelJob( new DeleteJobEvent(strFromUp, strDaemon, job_id_user) );
		m_ptrDaemonFSM->daemon_stage()->send(pEvtDelJob);

		// wait for an acknowledgment from Orchestrator that job was deleted
		sdpa::job_id_t jobid = pTestStr->WaitForEvent<sdpa::events::DeleteJobAckEvent>(pErrorEvt)->job_id();
		os.str("");
		os<<"Successfully deleted the job "<<jobid;
		SDPA_LOG_DEBUG(os.str());
	}
	else
		SDPA_LOG_ERROR("The job is supposed to be into a 'terminal state' in order to be able to retrieve results!");

	}

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtInt( new InterruptEvent(strDaemon, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtInt);

	// you can leave now
	SDPA_LOG_DEBUG("Slave: Finished!");
}


void DaemonFSMTest::testDaemonFSM_JobCancelled()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobCancelled******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string strFromUp("user");
	string strFromDown("aggregator");
	string strDaemon   = m_ptrDaemonFSM->name();
	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	TestStrategy* pTestStr = dynamic_cast<TestStrategy*>(m_ptrTestStrategy.get());

    sdpa::util::time_type start(sdpa::util::now());
    //start-up the orchestrator
    StartUpEvent::Ptr pEvtStartUp(new StartUpEvent(strDaemon, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtStartUp);

	ConfigOkEvent::Ptr pEvtConfigOk( new ConfigOkEvent(strDaemon, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtConfigOk);

	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtWorkerReg);
	pTestStr->WaitForEvent<sdpa::events::WorkerRegistrationAckEvent>(pErrorEvt);

	ConfigRequestEvent::Ptr pEvtCfgReq( new ConfigRequestEvent(strFromDown, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtCfgReq);

	// wait for a configuration reply event
	pTestStr->WaitForEvent<sdpa::events::ConfigReplyEvent>(pErrorEvt);

	LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtLS);

	for(int k=0; k<NITER;k++) {
	// the user submits a job
	SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUp, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtSubmitJob);

	// the user waits for an acknowledgment
	sdpa::job_id_t job_id_user = pTestStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

	// the slave posts a job request
	RequestJobEvent::Ptr pEvtReq( new RequestJobEvent(strFromDown, strDaemon) );
	m_ptrDaemonFSM->daemon_stage()->send(pEvtReq);

	sdpa::job_id_t job_id_slave;
	SubmitJobEvent::Ptr pSubmitJobEvent = pTestStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
	while( pErrorEvt.get() )
	{
		if(pErrorEvt->error_code() == ErrorEvent::SDPA_ENOJOBAVAIL)
		{
			os.str("");
			os<<"No job available! Try again ...";
			SDPA_LOG_DEBUG(os.str());

			//Post new reqest and wait
			RequestJobEvent::Ptr pEvtReqNew( new RequestJobEvent(strFromDown, strDaemon) );
			m_ptrDaemonFSM->daemon_stage()->send(pEvtReqNew);
			// wait the master to reply to the job request
			pErrorEvt.reset();
			pSubmitJobEvent = pTestStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
		}
	}

	SDPA_LOG_DEBUG("Try to set the JobId ...");
	job_id_slave = pSubmitJobEvent->job_id();

	// send a SubmitJobAckEvent to master
	// the master should acknowledge the job then
	SubmitJobAckEvent::Ptr pSubmitJobAck( new SubmitJobAckEvent(strFromDown, strDaemon, job_id_slave) );
	m_ptrDaemonFSM->daemon_stage()->send(pSubmitJobAck);

	// Now, a cancel message the user sends a CancelJob  message
	CancelJobEvent::Ptr pCancelJobEvt( new CancelJobEvent(strFromUp, strDaemon, job_id_user) );
	m_ptrDaemonFSM->daemon_stage()->send(pCancelJobEvt);

	// the worker expects a CancelJobEvent ...
	CancelJobEvent::Ptr pCancelEvt = pTestStr->WaitForEvent<sdpa::events::CancelJobEvent>(pErrorEvt);

	// the worker cancells the job
	SDPA_LOG_DEBUG("SLAVE: Canceled the job "<<pCancelEvt->job_id()<<"!Sending CancelJobAckEvent to "<<pCancelEvt->from()<<" ...");

	// ... and replies with a CancelJobAckEvent
	CancelJobAckEvent::Ptr pCancelJobAckEvt(new CancelJobAckEvent(pCancelEvt->to(), pCancelEvt->from(), pCancelEvt->job_id()));
	m_ptrDaemonFSM->daemon_stage()->send(pCancelJobAckEvt);

	// the user expects now a CancelJobAckEvent

	CancelJobAckEvent::Ptr pCancelAckEvt = pTestStr->WaitForEvent<sdpa::events::CancelJobAckEvent>(pErrorEvt);
	SDPA_LOG_DEBUG("USER: The job "<<pCancelAckEvt->job_id()<<" has been successfully cancelled!");

	}

	sleep(2);
	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtInt( new InterruptEvent(strDaemon, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtInt);

	// you can leave now
	SDPA_LOG_DEBUG("Slave: Finished!");
}



void DaemonFSMTest::testDaemonFSM_JobCancelled_from_Pending()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobCancelled_from_Pending******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string strFromUp("user");
	string strFromDown("aggregator");
	string strDaemon   = m_ptrDaemonFSM->name();
	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	TestStrategy* pTestStr = dynamic_cast<TestStrategy*>(m_ptrTestStrategy.get());

    sdpa::util::time_type start(sdpa::util::now());
    //start-up the orchestrator
    StartUpEvent::Ptr pEvtStartUp(new StartUpEvent(strDaemon, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtStartUp);

	ConfigOkEvent::Ptr pEvtConfigOk( new ConfigOkEvent(strDaemon, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtConfigOk);

	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtWorkerReg);
	pTestStr->WaitForEvent<sdpa::events::WorkerRegistrationAckEvent>(pErrorEvt);

	ConfigRequestEvent::Ptr pEvtCfgReq( new ConfigRequestEvent(strFromDown, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtCfgReq);

	// wait for a configuration reply event
	pTestStr->WaitForEvent<sdpa::events::ConfigReplyEvent>(pErrorEvt);

	LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtLS);

	for(int k=0; k<NITER; k++ ) {
	// the user submits a job
	SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUp, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtSubmitJob);

	// the user waits for an acknowledgment
	sdpa::job_id_t job_id_user = pTestStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

	// Now, a cancel message the user sends a CancelJob  message
	CancelJobEvent::Ptr pCancelJobEvt( new CancelJobEvent(strFromUp, strDaemon, job_id_user) );
	m_ptrDaemonFSM->daemon_stage()->send(pCancelJobEvt);

	// the user expects now a CancelJobAckEvent
	CancelJobAckEvent::Ptr pCancelAckEvt = pTestStr->WaitForEvent<sdpa::events::CancelJobAckEvent>(pErrorEvt);
	SDPA_LOG_DEBUG("User: The job "<<pCancelAckEvt->job_id()<<" has been successfully cancelled!");
	}

	sleep(2);
	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtInt( new InterruptEvent(strDaemon, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtInt);

	// you can leave now
	SDPA_LOG_DEBUG("Slave: Finished!");
}
