#include "test_DaemonRealGwes.hpp"

#include <iostream>
#include <string>
#include <list>
#include <sdpa/memory.hpp>
#include <time.h>
#include <sdpa/util/util.hpp>
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

#include <gwes/GWES.h>

#include <fhglog/Configuration.hpp>
#include <seda/Stage.hpp>
#include <seda/StageRegistry.hpp>

#include <iostream>
#include <fstream>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::events;
using namespace sdpa::daemon;
using namespace sdpa::fsm::smc;

const int NITER = 100;

const int sleep_interval = 1000; //us

class TestStrategy : public seda::Strategy
{
public:
	 typedef std::tr1::shared_ptr<TestStrategy> Ptr;
	 TestStrategy(const std::string& name): seda::Strategy(name), SDPA_INIT_LOGGER(name)  {}
	 void perform(const seda::IEvent::Ptr& pEvt)
	 {

		 if( dynamic_cast<WorkerRegistrationAckEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received WorkerRegistrationAckEvent!");
		 }
		 else if( dynamic_cast<ConfigReplyEvent*>(pEvt.get())  )
		 {
			 SDPA_LOG_DEBUG("Received ConfigReplyEvent!");
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
		     SDPA_LOG_DEBUG("Received unexpected event of type "<<typeid(*pEvt).name()<<"!");
		 }

		 eventQueue.push(pEvt);
	 }

	 std::string str() const
	 {
		std::ostringstream ostream(std::ostringstream::out);
		eventQueue_t::const_iterator it;
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

CPPUNIT_TEST_SUITE_REGISTRATION( DaemonRealGwesTest );

DaemonRealGwesTest::DaemonRealGwesTest() :
	SDPA_INIT_LOGGER("sdpa.tests.DaemonRealGwesTest")
{
}

DaemonRealGwesTest::~DaemonRealGwesTest()
{}


string DaemonRealGwesTest::read_workflow(string strFileName)
{
	ifstream f(strFileName.c_str());
	ostringstream os;
	os.str("");

	if( f.is_open() )
	{
		char c;
		while (f.get(c)) os<<c;
		f.close();
	}else
		cout<<"Unable to open file " << strFileName << ", error: " <<strerror(errno);

	return os.str();
}

void DaemonRealGwesTest::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	m_ptrSdpa2Gwes = new gwes::GWES();

	m_ptrMasterStrategy = seda::Strategy::Ptr( new TestStrategy("User") );
	m_ptrToMasterStage = seda::Stage::Ptr(new seda::Stage("to_master_stage", m_ptrMasterStrategy) );

	m_ptrSlaveStrategy = seda::Strategy::Ptr( new TestStrategy("Slave") );
	m_ptrToSlaveStage = seda::Stage::Ptr(new seda::Stage("to_slave_stage", m_ptrSlaveStrategy) );

	m_ptrDaemonFSM = DaemonFSM::ptr_t(new DaemonFSM( sdpa::daemon::ORCHESTRATOR, m_ptrToMasterStage.get(),
													 m_ptrToSlaveStage.get(), m_ptrSdpa2Gwes));

	DaemonFSM::create_daemon_stage(m_ptrDaemonFSM);
	DaemonFSM::start(m_ptrDaemonFSM);

	//create output stage
	seda::StageRegistry::instance().insert(m_ptrToMasterStage);
	m_ptrToMasterStage->start();

	seda::StageRegistry::instance().insert(m_ptrToSlaveStage);
	m_ptrToSlaveStage->start();

	m_strWorkflow = read_workflow("workflows/masterworkflow-sdpa-test.gwdl");
	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);
}

void DaemonRealGwesTest::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	seda::StageRegistry::instance().lookup(m_ptrToMasterStage->name())->stop();
	seda::StageRegistry::instance().lookup(m_ptrToSlaveStage->name())->stop();
	m_ptrDaemonFSM->stop();

	seda::StageRegistry::instance().clear();

	//m_ptrDaemonFSM.reset();
	delete m_ptrSdpa2Gwes;
}

void DaemonRealGwesTest::testDaemonFSM_JobFinished()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobFinished******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string strFromUp("user");
	string strFromDown("aggregator");
	string strDaemon   = m_ptrDaemonFSM->name();
	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	TestStrategy* pUserStr = dynamic_cast<TestStrategy*>(m_ptrMasterStrategy.get());
	TestStrategy* pSlaveStr = dynamic_cast<TestStrategy*>(m_ptrSlaveStrategy.get());

	SDPA_LOG_DEBUG("Slave: send  WorkerRegistrationEvent to "<<strDaemon);
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtWorkerReg);
	pSlaveStr->WaitForEvent<sdpa::events::WorkerRegistrationAckEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: request configuration from "<<strDaemon);
	ConfigRequestEvent::Ptr pEvtCfgReq( new ConfigRequestEvent(strFromDown, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtCfgReq);

	// wait for a configuration reply event
	pSlaveStr->WaitForEvent<sdpa::events::ConfigReplyEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: send LS to "<<strDaemon);
	LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtLS);

	for(int k=0; k<NITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<strDaemon);
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUp, strDaemon, "", m_strWorkflow));
		m_ptrDaemonFSM->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		// the slave posts a job request
		SDPA_LOG_DEBUG("Slave: post new job request to "<<strDaemon);
		RequestJobEvent::Ptr pEvtReq( new RequestJobEvent(strFromDown, strDaemon) );
		m_ptrDaemonFSM->daemon_stage()->send(pEvtReq);

		sdpa::job_id_t job_id_slave;
		SubmitJobEvent::Ptr pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
		while( pErrorEvt.get() )
		{
			if(pErrorEvt->error_code() == ErrorEvent::SDPA_ENOJOBAVAIL)
			{
				os.str("");
				os<<"No job available! Try again ...";
				SDPA_LOG_DEBUG(os.str());

				usleep(sleep_interval);
				//Post new reqest and wait
				RequestJobEvent::Ptr pEvtReqNew( new RequestJobEvent(strFromDown, strDaemon) );
				m_ptrDaemonFSM->daemon_stage()->send(pEvtReqNew);
				// wait the master to reply to the job request
				pErrorEvt.reset();
				pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
			}
		}

		job_id_slave = pSubmitJobEvent->job_id();

		// send a SubmitJobAckEvent to master
		// the master should acknowledge the job then
		SDPA_LOG_DEBUG("Slave: send SubmitJobAckEvent to "<<strDaemon);
		SubmitJobAckEvent::Ptr pSubmitJobAck( new SubmitJobAckEvent(strFromDown, strDaemon, job_id_slave) );
		m_ptrDaemonFSM->daemon_stage()->send(pSubmitJobAck);

		// the slave computes the job ........

		// submit a JobFinishedEvent to master
		SDPA_LOG_DEBUG("Slave: send JobFinishedEvent to "<<strDaemon);
		JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(strFromDown, strDaemon, job_id_slave));
		m_ptrDaemonFSM->daemon_stage()->send(pEvtJobFinished);

		pSlaveStr->WaitForEvent<sdpa::events::JobFinishedAckEvent>(pErrorEvt);

		// check if the job finished
		SDPA_LOG_DEBUG("User: query "<<strDaemon<<" for the status of the job "<<job_id_user);
		QueryJobStatusEvent::Ptr pEvtQueryStatus(new QueryJobStatusEvent(strFromUp, strDaemon, job_id_user));
		m_ptrDaemonFSM->daemon_stage()->send(pEvtQueryStatus);

		// wait for a JobStatusReplyEvent
		JobStatusReplyEvent::Ptr pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());

		while( pJobStatusReplyEvent->status().find("Finished") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Failed") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Cancelled") == std::string::npos )
		{
			SDPA_LOG_DEBUG("User: ask the "<<strDaemon<<" for the status of the job "<<job_id_user);
			QueryJobStatusEvent::Ptr pEvtQueryStNew(new QueryJobStatusEvent(strFromUp, strDaemon, job_id_user));
			m_ptrDaemonFSM->daemon_stage()->send(pEvtQueryStNew);

			// wait for a JobStatusReplyEvent
			pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());
			usleep(sleep_interval);
		}

		// if the job is in the finished or failed state, one is allowed
		// to retriieve the results now
		SDPA_LOG_DEBUG("User: retrieve the results of the job "<<job_id_user);
		RetrieveJobResultsEvent::Ptr pEvtRetrieveRes(new RetrieveJobResultsEvent(strFromUp, strDaemon, job_id_user));
		m_ptrDaemonFSM->daemon_stage()->send(pEvtRetrieveRes);
		// wait for a JobStatusReplyEvent
		pUserStr->WaitForEvent<sdpa::events::JobResultsReplyEvent>(pErrorEvt);

		// check the job status. if the job is in a final state, send a DeletJobEvent
		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		DeleteJobEvent::Ptr pEvtDelJob( new DeleteJobEvent(strFromUp, strDaemon, job_id_user) );
		m_ptrDaemonFSM->daemon_stage()->send(pEvtDelJob);

		// wait for an acknowledgment from Orchestrator that job was deleted
		sdpa::job_id_t jobid = pUserStr->WaitForEvent<sdpa::events::DeleteJobAckEvent>(pErrorEvt)->job_id();
		os.str("");
		os<<"Successfully deleted the job "<<jobid;
		SDPA_LOG_DEBUG(os.str());
	}

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtInt( new InterruptEvent(strDaemon, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtInt);

	// you can leave now
	SDPA_LOG_DEBUG("Slave: Finished!");
}

void DaemonRealGwesTest::testDaemonFSM_JobFailed()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobFailed******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string strFromUp("user");
	string strFromDown("aggregator");
	string strDaemon   = m_ptrDaemonFSM->name();
	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	TestStrategy* pUserStr = dynamic_cast<TestStrategy*>(m_ptrMasterStrategy.get());
	TestStrategy* pSlaveStr = dynamic_cast<TestStrategy*>(m_ptrSlaveStrategy.get());

	SDPA_LOG_DEBUG("Slave: send  WorkerRegistrationEvent to "<<strDaemon);
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtWorkerReg);
	pSlaveStr->WaitForEvent<sdpa::events::WorkerRegistrationAckEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: request configuration from "<<strDaemon);
	ConfigRequestEvent::Ptr pEvtCfgReq( new ConfigRequestEvent(strFromDown, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtCfgReq);

	// wait for a configuration reply event
	pSlaveStr->WaitForEvent<sdpa::events::ConfigReplyEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: send LS to "<<strDaemon);
	LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtLS);

	for(int k=0; k<NITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<strDaemon);
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUp, strDaemon, "", m_strWorkflow));
		m_ptrDaemonFSM->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		// the slave posts a job request
		SDPA_LOG_DEBUG("Slave: post new job request to "<<strDaemon);
		RequestJobEvent::Ptr pEvtReq( new RequestJobEvent(strFromDown, strDaemon) );
		m_ptrDaemonFSM->daemon_stage()->send(pEvtReq);

		sdpa::job_id_t job_id_slave;
		SubmitJobEvent::Ptr pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
		while( pErrorEvt.get() )
		{
			if(pErrorEvt->error_code() == ErrorEvent::SDPA_ENOJOBAVAIL)
			{
				os.str("");
				os<<"No job available! Try again ...";
				SDPA_LOG_DEBUG(os.str());

				usleep(sleep_interval);
				//Post new reqest and wait
				RequestJobEvent::Ptr pEvtReqNew( new RequestJobEvent(strFromDown, strDaemon) );
				m_ptrDaemonFSM->daemon_stage()->send(pEvtReqNew);
				// wait the master to reply to the job request
				pErrorEvt.reset();
				pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
			}
		}

		job_id_slave = pSubmitJobEvent->job_id();

		// send a SubmitJobAckEvent to master
		// the master should acknowledge the job then
		SDPA_LOG_DEBUG("Slave: send SubmitJobAckEvent to "<<strDaemon);
		SubmitJobAckEvent::Ptr pSubmitJobAck( new SubmitJobAckEvent(strFromDown, strDaemon, job_id_slave) );
		m_ptrDaemonFSM->daemon_stage()->send(pSubmitJobAck);

		// the slave computes the job ........

		// submit a JobFinishedEvent to master
		SDPA_LOG_DEBUG("Slave: send JobFailedEvent to "<<strDaemon);
		JobFailedEvent::Ptr pEvtJobFailed(new JobFailedEvent(strFromDown, strDaemon, job_id_slave));
		m_ptrDaemonFSM->daemon_stage()->send(pEvtJobFailed);

		pSlaveStr->WaitForEvent<sdpa::events::JobFailedAckEvent>(pErrorEvt);

		// the user checks the status of his job
		SDPA_LOG_DEBUG("User: query "<<strDaemon<<" for the status of the job "<<job_id_user);
		QueryJobStatusEvent::Ptr pEvtQueryStatus(new QueryJobStatusEvent(strFromUp, strDaemon, job_id_user));
		m_ptrDaemonFSM->daemon_stage()->send(pEvtQueryStatus);

		// wait for a JobStatusReplyEvent
		JobStatusReplyEvent::Ptr pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());

		while( pJobStatusReplyEvent->status().find("Finished") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Failed") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Cancelled") == std::string::npos )
		{
			SDPA_LOG_DEBUG("User: ask the "<<strDaemon<<" for the status of the job "<<job_id_user);
			QueryJobStatusEvent::Ptr pEvtQueryStNew(new QueryJobStatusEvent(strFromUp, strDaemon, job_id_user));
			m_ptrDaemonFSM->daemon_stage()->send(pEvtQueryStNew);

			// wait for a JobStatusReplyEvent
			pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());
			usleep(sleep_interval);
		}

		// if the job is in the finished or failed state, one is allowed
		// to retriieve the results now
		SDPA_LOG_DEBUG("User: retrieve the results of the job "<<job_id_user);
		RetrieveJobResultsEvent::Ptr pEvtRetrieveRes(new RetrieveJobResultsEvent(strFromUp, strDaemon, job_id_user));
		m_ptrDaemonFSM->daemon_stage()->send(pEvtRetrieveRes);
		// wait for a JobStatusReplyEvent
		pUserStr->WaitForEvent<sdpa::events::JobResultsReplyEvent>(pErrorEvt);

		// check the job status. if the job is in a final state, send a DeletJobEvent
		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		DeleteJobEvent::Ptr pEvtDelJob( new DeleteJobEvent(strFromUp, strDaemon, job_id_user) );
		m_ptrDaemonFSM->daemon_stage()->send(pEvtDelJob);

		// wait for an acknowledgment from Orchestrator that job was deleted
		sdpa::job_id_t jobid = pUserStr->WaitForEvent<sdpa::events::DeleteJobAckEvent>(pErrorEvt)->job_id();
		os.str("");
		os<<"Successfully deleted the job "<<jobid;
		SDPA_LOG_DEBUG(os.str());
	}

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtInt( new InterruptEvent(strDaemon, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtInt);

	// you can leave now
	SDPA_LOG_DEBUG("Slave: Finished!");
}


void DaemonRealGwesTest::testDaemonFSM_JobCancelled()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobCancelled******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string strFromUp("user");
	string strFromDown("aggregator");
	string strDaemon   = m_ptrDaemonFSM->name();
	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	TestStrategy* pUserStr = dynamic_cast<TestStrategy*>(m_ptrMasterStrategy.get());
	TestStrategy* pSlaveStr = dynamic_cast<TestStrategy*>(m_ptrSlaveStrategy.get());

	SDPA_LOG_DEBUG("Slave: send  WorkerRegistrationEvent to "<<strDaemon);
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtWorkerReg);
	pSlaveStr->WaitForEvent<sdpa::events::WorkerRegistrationAckEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: request configuration from "<<strDaemon);
	ConfigRequestEvent::Ptr pEvtCfgReq( new ConfigRequestEvent(strFromDown, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtCfgReq);

	// wait for a configuration reply event
	pSlaveStr->WaitForEvent<sdpa::events::ConfigReplyEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: send LS to "<<strDaemon);
	LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtLS);

	for(int k=0; k<NITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<strDaemon);
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUp, strDaemon, "", m_strWorkflow));
		m_ptrDaemonFSM->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		// the slave posts a job request
		SDPA_LOG_DEBUG("Slave: post new job request to "<<strDaemon);
		RequestJobEvent::Ptr pEvtReq( new RequestJobEvent(strFromDown, strDaemon) );
		m_ptrDaemonFSM->daemon_stage()->send(pEvtReq);

		sdpa::job_id_t job_id_slave;
		SubmitJobEvent::Ptr pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
		while( pErrorEvt.get() )
		{
			if(pErrorEvt->error_code() == ErrorEvent::SDPA_ENOJOBAVAIL)
			{
				os.str("");
				os<<"No job available! Try again ...";
				SDPA_LOG_DEBUG(os.str());

				usleep(sleep_interval);
				//Post new reqest and wait
				RequestJobEvent::Ptr pEvtReqNew( new RequestJobEvent(strFromDown, strDaemon) );
				m_ptrDaemonFSM->daemon_stage()->send(pEvtReqNew);
				// wait the master to reply to the job request
				pErrorEvt.reset();
				pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
			}
		}

		job_id_slave = pSubmitJobEvent->job_id();

		// send a SubmitJobAckEvent to master
		// the master should acknowledge the job then
		SDPA_LOG_DEBUG("Slave: send SubmitJobAckEvent to "<<strDaemon);
		SubmitJobAckEvent::Ptr pSubmitJobAck( new SubmitJobAckEvent(strFromDown, strDaemon, job_id_slave) );
		m_ptrDaemonFSM->daemon_stage()->send(pSubmitJobAck);

		usleep(sleep_interval);
		// Now, the user sends a CancelJob  message
		CancelJobEvent::Ptr pCancelJobEvt( new CancelJobEvent(strFromUp, strDaemon, job_id_user) );
		m_ptrDaemonFSM->daemon_stage()->send(pCancelJobEvt);

		// the worker expects a CancelJobEvent ...
		CancelJobEvent::Ptr pCancelEvt = pSlaveStr->WaitForEvent<sdpa::events::CancelJobEvent>(pErrorEvt);

		// the worker cancells the job
		SDPA_LOG_DEBUG("SLAVE: Canceled the job "<<pCancelEvt->job_id()<<"!Sending ack to "<<pCancelEvt->from());

		// ... and replies with a CancelJobAckEvent
		CancelJobAckEvent::Ptr pCancelJobAckEvt(new CancelJobAckEvent(pCancelEvt->to(), pCancelEvt->from(), pCancelEvt->job_id()));
		m_ptrDaemonFSM->daemon_stage()->send(pCancelJobAckEvt);

		// the user expects now a CancelJobAckEvent
		CancelJobAckEvent::Ptr pCancelAckEvt = pUserStr->WaitForEvent<sdpa::events::CancelJobAckEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("USER: The job "<<pCancelAckEvt->job_id()<<" has been successfully cancelled!");
	}

	sleep(2);
	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtInt( new InterruptEvent(strDaemon, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtInt);

	// you can leave now
	SDPA_LOG_DEBUG("Slave: Finished!");
}

void DaemonRealGwesTest::testDaemonFSM_JobCancelled_from_Pending()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobCancelled_from_Pending******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string strFromUp("user");
	string strFromDown("aggregator");
	string strDaemon   = m_ptrDaemonFSM->name();
	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	TestStrategy* pUserStr = dynamic_cast<TestStrategy*>(m_ptrMasterStrategy.get());
	TestStrategy* pSlaveStr = dynamic_cast<TestStrategy*>(m_ptrSlaveStrategy.get());

	SDPA_LOG_DEBUG("Slave: send  WorkerRegistrationEvent to "<<strDaemon);
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtWorkerReg);
	pSlaveStr->WaitForEvent<sdpa::events::WorkerRegistrationAckEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: request configuration from "<<strDaemon);
	ConfigRequestEvent::Ptr pEvtCfgReq( new ConfigRequestEvent(strFromDown, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtCfgReq);

	// wait for a configuration reply event
	pSlaveStr->WaitForEvent<sdpa::events::ConfigReplyEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: send LS to "<<strDaemon);
	LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromDown, strDaemon));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtLS);

	for(int k=0; k<NITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<strDaemon);
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUp, strDaemon, "", m_strWorkflow));
		m_ptrDaemonFSM->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		usleep(sleep_interval);
		// Now, the user sends a CancelJob  message
		CancelJobEvent::Ptr pCancelJobEvt( new CancelJobEvent(strFromUp, strDaemon, job_id_user) );
		m_ptrDaemonFSM->daemon_stage()->send(pCancelJobEvt);

		// the user expects a CancelJobAckEvent
		CancelJobAckEvent::Ptr pCancelAckEvt = pUserStr->WaitForEvent<sdpa::events::CancelJobAckEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("USER: The job "<<pCancelAckEvt->job_id()<<" has been successfully cancelled!");
	}

	sleep(2);
	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtInt( new InterruptEvent(strDaemon, strDaemon ));
	m_ptrDaemonFSM->daemon_stage()->send(pEvtInt);

	// you can leave now
	SDPA_LOG_DEBUG("Slave: Finished!");
}
