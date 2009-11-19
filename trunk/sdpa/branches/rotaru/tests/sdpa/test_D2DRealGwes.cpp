/*
 * =====================================================================================
 *
 *       Filename:  test_D2DRealGwes.cpp
 *
 *    Description:  test 2 generic daemons, each with a real gwes
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#include "test_D2DRealGwes.hpp"

#include <iostream>
#include <string>
#include <list>
#include <sdpa/memory.hpp>
#include <time.h>
#include <sdpa/util/util.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

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

#include <gwes/GWES.h>

#include <seda/Stage.hpp>
#include <seda/StageRegistry.hpp>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::events;
using namespace sdpa::daemon;


const int NITER = 1;
const int sleep_interval = 10000;

class TestStrategy : public seda::Strategy
{
public:
     typedef sdpa::shared_ptr<TestStrategy> Ptr;
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

CPPUNIT_TEST_SUITE_REGISTRATION( D2DRealGwesTest );

D2DRealGwesTest::D2DRealGwesTest() :
	SDPA_INIT_LOGGER("sdpa.tests.D2DRealGwesTest")
{
}

D2DRealGwesTest::~D2DRealGwesTest()
{}


string D2DRealGwesTest::read_workflow(string strFileName)
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

void D2DRealGwesTest::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	m_ptrSdpa2GwesOrch = new gwes::GWES();
	m_ptrSdpa2GwesAgg  = new gwes::GWES();

	m_ptrUserStrategy = seda::Strategy::Ptr( new TestStrategy("User") );
	m_ptrToUserStage = seda::Stage::Ptr(new seda::Stage("to_master_stage", m_ptrUserStrategy) );

	m_ptrNreStrategy = seda::Strategy::Ptr( new TestStrategy("Nre") );
	m_ptrToNreStage = seda::Stage::Ptr(new seda::Stage("to_nre_stage", m_ptrNreStrategy) );

	m_ptrOrch = DaemonFSM::ptr_t(new DaemonFSM( sdpa::daemon::ORCHESTRATOR,
												m_ptrToUserStage.get(),
												NULL,
												m_ptrSdpa2GwesOrch));
	DaemonFSM::create_daemon_stage(m_ptrOrch);


	m_ptrAgg = DaemonFSM::ptr_t(new DaemonFSM( sdpa::daemon::AGGREGATOR,
								m_ptrOrch->daemon_stage(),
								m_ptrToNreStage.get(),
								m_ptrSdpa2GwesAgg));
	DaemonFSM::create_daemon_stage(m_ptrAgg);


	m_ptrOrch->set_to_slave_stage(m_ptrAgg->daemon_stage());


	DaemonFSM::start(m_ptrOrch);
	DaemonFSM::start(m_ptrAgg);

	//create output stage
	seda::StageRegistry::instance().insert(m_ptrToUserStage);
	m_ptrToUserStage->start();

	seda::StageRegistry::instance().insert(m_ptrToNreStage);
	m_ptrToNreStage->start();

	m_strWorkflow = read_workflow("workflows/masterworkflow-sdpa-test.gwdl");
	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);
}

void D2DRealGwesTest::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	seda::StageRegistry::instance().lookup(m_ptrToUserStage->name())->stop();
	seda::StageRegistry::instance().lookup(m_ptrToNreStage->name())->stop();

	m_ptrAgg->stop();
	m_ptrOrch->stop();

	seda::StageRegistry::instance().clear();

	//m_ptrOrch.reset();
	delete m_ptrSdpa2GwesOrch;
	delete m_ptrSdpa2GwesAgg;
}

void D2DRealGwesTest::testDaemonFSM_JobFinished()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobFinished******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string strFromUser(sdpa::daemon::USER);
	string strFromNre(sdpa::daemon::NRE);


	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	TestStrategy* pUserStr = dynamic_cast<TestStrategy*>(m_ptrUserStrategy.get());
	TestStrategy* pSlaveStr = dynamic_cast<TestStrategy*>(m_ptrNreStrategy.get());


	SDPA_LOG_DEBUG("Slave: send  WorkerRegistrationEvent to "<<m_ptrAgg->name());
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromNre, m_ptrAgg->name()));
	m_ptrAgg->daemon_stage()->send(pEvtWorkerReg);
	pSlaveStr->WaitForEvent<sdpa::events::WorkerRegistrationAckEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: request configuration from "<<m_ptrAgg->name());
	ConfigRequestEvent::Ptr pEvtCfgReq( new ConfigRequestEvent(strFromNre, m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtCfgReq);

	// wait for a configuration reply event
	pSlaveStr->WaitForEvent<sdpa::events::ConfigReplyEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: send LS to "<<m_ptrAgg->name());
	LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromNre, m_ptrAgg->name()));
	m_ptrAgg->daemon_stage()->send(pEvtLS);

	for(int k=0; k<NITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<m_ptrOrch->name());
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUser, m_ptrOrch->name(), "", m_strWorkflow));
		m_ptrOrch->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		// the slave posts a job request
		SDPA_LOG_DEBUG("Slave: post new job request to "<<m_ptrAgg->name());
		RequestJobEvent::Ptr pEvtReq( new RequestJobEvent(strFromNre, m_ptrAgg->name()) );
		m_ptrAgg->daemon_stage()->send(pEvtReq);

		sdpa::job_id_t job_id_slave;
		SubmitJobEvent::Ptr pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
		while( pErrorEvt.get() )
		{
			if(pErrorEvt->error_code() == ErrorEvent::SDPA_ENOJOBAVAIL)
			{
				os.str("");
				os<<"No job available! Try again ...";
				SDPA_LOG_DEBUG(os.str());

				sleep(1);
				//Post new reqest and wait
				RequestJobEvent::Ptr pEvtReqNew( new RequestJobEvent(strFromNre, m_ptrAgg->name()) );
				m_ptrAgg->daemon_stage()->send(pEvtReqNew);
				// wait the master to reply to the job request
				pErrorEvt.reset();
				pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
			}
		}

		job_id_slave = pSubmitJobEvent->job_id();

		// send a SubmitJobAckEvent to master
		// the master should acknowledge the job then
		SDPA_LOG_DEBUG("Slave: send SubmitJobAckEvent to "<<m_ptrAgg->name());
		SubmitJobAckEvent::Ptr pSubmitJobAck( new SubmitJobAckEvent(strFromNre, m_ptrAgg->name(), job_id_slave) );
		m_ptrAgg->daemon_stage()->send(pSubmitJobAck);

		// the slave computes the job ........

		// submit a JobFinishedEvent to master
		sdpa::job_result_t result;
		SDPA_LOG_DEBUG("Slave: send JobFinishedEvent to "<<m_ptrAgg->name());
		JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(strFromNre, m_ptrAgg->name(), job_id_slave, result));
		m_ptrAgg->daemon_stage()->send(pEvtJobFinished);

		pSlaveStr->WaitForEvent<sdpa::events::JobFinishedAckEvent>(pErrorEvt);

		// check if the job finished
		SDPA_LOG_DEBUG("User: query "<<m_ptrOrch->name()<<" for the status of the job "<<job_id_user);
		QueryJobStatusEvent::Ptr pEvtQueryStatus(new QueryJobStatusEvent(strFromUser, m_ptrOrch->name(), job_id_user));
		m_ptrOrch->daemon_stage()->send(pEvtQueryStatus);

		// wait for a JobStatusReplyEvent
		JobStatusReplyEvent::Ptr pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());

		while( pJobStatusReplyEvent->status().find("Finished") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Failed") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Cancelled") == std::string::npos)
		{
			SDPA_LOG_DEBUG("User: ask the "<<m_ptrOrch->name()<<" for the status of the job "<<job_id_user);
			QueryJobStatusEvent::Ptr pEvtQueryStNew(new QueryJobStatusEvent(strFromUser, m_ptrOrch->name(), job_id_user));
			m_ptrOrch->daemon_stage()->send(pEvtQueryStNew);

			// wait for a JobStatusReplyEvent
			pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());
			sleep(1);
		}

		// if the job is in the finished or failed state, one is allowed
		// to retriieve the results now
		SDPA_LOG_DEBUG("User: retrieve the results of the job "<<job_id_user);
		RetrieveJobResultsEvent::Ptr pEvtRetrieveRes(new RetrieveJobResultsEvent(strFromUser, m_ptrOrch->name(), job_id_user));
		m_ptrOrch->daemon_stage()->send(pEvtRetrieveRes);
		// wait for a JobStatusReplyEvent
		pUserStr->WaitForEvent<sdpa::events::JobResultsReplyEvent>(pErrorEvt);

		// check the job status. if the job is in a final state, send a DeletJobEvent
		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		DeleteJobEvent::Ptr pEvtDelJob( new DeleteJobEvent(strFromUser, m_ptrOrch->name(), job_id_user) );
		m_ptrOrch->daemon_stage()->send(pEvtDelJob);

		// wait for an acknowledgment from Orchestrator that job was deleted
		sdpa::job_id_t jobid = pUserStr->WaitForEvent<sdpa::events::DeleteJobAckEvent>(pErrorEvt)->job_id();
		os.str("");
		os<<"Successfully deleted the job "<<jobid;
		SDPA_LOG_DEBUG(os.str());
	}


	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);


	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);

	// you can leave now
	SDPA_LOG_DEBUG("Slave: Finished!");
}

void D2DRealGwesTest::testDaemonFSM_JobFailed()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobFailed******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string strFromUser(sdpa::daemon::USER);
	string strFromNre(sdpa::daemon::NRE);


	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	TestStrategy* pUserStr = dynamic_cast<TestStrategy*>(m_ptrUserStrategy.get());
	TestStrategy* pSlaveStr = dynamic_cast<TestStrategy*>(m_ptrNreStrategy.get());


	SDPA_LOG_DEBUG("Slave: send  WorkerRegistrationEvent to "<<m_ptrAgg->name());
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromNre, m_ptrAgg->name()));
	m_ptrAgg->daemon_stage()->send(pEvtWorkerReg);
	pSlaveStr->WaitForEvent<sdpa::events::WorkerRegistrationAckEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: request configuration from "<<m_ptrAgg->name());
	ConfigRequestEvent::Ptr pEvtCfgReq( new ConfigRequestEvent(strFromNre, m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtCfgReq);

	// wait for a configuration reply event
	pSlaveStr->WaitForEvent<sdpa::events::ConfigReplyEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: send LS to "<<m_ptrAgg->name());
	LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromNre, m_ptrAgg->name()));
	m_ptrAgg->daemon_stage()->send(pEvtLS);

	for(int k=0; k<NITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<m_ptrOrch->name());
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUser, m_ptrOrch->name(), "", m_strWorkflow));
		m_ptrOrch->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		// the slave posts a job request
		SDPA_LOG_DEBUG("Slave: post new job request to "<<m_ptrAgg->name());
		RequestJobEvent::Ptr pEvtReq( new RequestJobEvent(strFromNre, m_ptrAgg->name()) );
		m_ptrAgg->daemon_stage()->send(pEvtReq);

		sdpa::job_id_t job_id_slave;
		SubmitJobEvent::Ptr pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
		while( pErrorEvt.get() )
		{
			if(pErrorEvt->error_code() == ErrorEvent::SDPA_ENOJOBAVAIL)
			{
				os.str("");
				os<<"No job available! Try again ...";
				SDPA_LOG_DEBUG(os.str());

				sleep(1);
				//Post new reqest and wait
				RequestJobEvent::Ptr pEvtReqNew( new RequestJobEvent(strFromNre, m_ptrAgg->name()) );
				m_ptrAgg->daemon_stage()->send(pEvtReqNew);
				// wait the master to reply to the job request
				pErrorEvt.reset();
				pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
			}
		}

		job_id_slave = pSubmitJobEvent->job_id();

		// send a SubmitJobAckEvent to master
		// the master should acknowledge the job then
		SDPA_LOG_DEBUG("Slave: send SubmitJobAckEvent to "<<m_ptrAgg->name());
		SubmitJobAckEvent::Ptr pSubmitJobAck( new SubmitJobAckEvent(strFromNre, m_ptrAgg->name(), job_id_slave) );
		m_ptrAgg->daemon_stage()->send(pSubmitJobAck);

		// the slave computes the job ........

		// submit a JobFinishedEvent to master
		sdpa::job_result_t result;
		SDPA_LOG_DEBUG("Slave: send JobFailedEvent to "<<m_ptrAgg->name());
		JobFailedEvent::Ptr pEvtJobFailed(new JobFailedEvent(strFromNre, m_ptrAgg->name(), job_id_slave, result));
		m_ptrAgg->daemon_stage()->send(pEvtJobFailed);

		pSlaveStr->WaitForEvent<sdpa::events::JobFailedAckEvent>(pErrorEvt);

		// check if the job finished
		SDPA_LOG_DEBUG("User: query "<<m_ptrOrch->name()<<" for the status of the job "<<job_id_user);
		QueryJobStatusEvent::Ptr pEvtQueryStatus(new QueryJobStatusEvent(strFromUser, m_ptrOrch->name(), job_id_user));
		m_ptrOrch->daemon_stage()->send(pEvtQueryStatus);

		// wait for a JobStatusReplyEvent
		JobStatusReplyEvent::Ptr pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());

		while( pJobStatusReplyEvent->status().find("Finished") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Failed") == std::string::npos &&
			pJobStatusReplyEvent->status().find("Cancelled") == std::string::npos)
		{
			SDPA_LOG_DEBUG("User: ask the "<<m_ptrOrch->name()<<" for the status of the job "<<job_id_user);
			QueryJobStatusEvent::Ptr pEvtQueryStNew(new QueryJobStatusEvent(strFromUser, m_ptrOrch->name(), job_id_user));
			m_ptrOrch->daemon_stage()->send(pEvtQueryStNew);

			// wait for a JobStatusReplyEvent
			pJobStatusReplyEvent = pUserStr->WaitForEvent<sdpa::events::JobStatusReplyEvent>(pErrorEvt);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<pJobStatusReplyEvent->status());
			sleep(1);
		}

		// if the job is in the finished or failed state, one is allowed
		// to retriieve the results now
		SDPA_LOG_DEBUG("User: retrieve the results of the job "<<job_id_user);
		RetrieveJobResultsEvent::Ptr pEvtRetrieveRes(new RetrieveJobResultsEvent(strFromUser, m_ptrOrch->name(), job_id_user));
		m_ptrOrch->daemon_stage()->send(pEvtRetrieveRes);
		// wait for a JobStatusReplyEvent
		pUserStr->WaitForEvent<sdpa::events::JobResultsReplyEvent>(pErrorEvt);

		// check the job status. if the job is in a final state, send a DeletJobEvent
		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		DeleteJobEvent::Ptr pEvtDelJob( new DeleteJobEvent(strFromUser, m_ptrOrch->name(), job_id_user) );
		m_ptrOrch->daemon_stage()->send(pEvtDelJob);

		// wait for an acknowledgment from Orchestrator that job was deleted
		sdpa::job_id_t jobid = pUserStr->WaitForEvent<sdpa::events::DeleteJobAckEvent>(pErrorEvt)->job_id();
		os.str("");
		os<<"Successfully deleted the job "<<jobid;
		SDPA_LOG_DEBUG(os.str());
	}

	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);

	// you can leave now
	SDPA_LOG_DEBUG("Slave: Finished!");
}

void D2DRealGwesTest::testDaemonFSM_JobCancelled()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobCancelled******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string strFromUser(sdpa::daemon::USER);
	string strFromNre(sdpa::daemon::NRE);


	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	TestStrategy* pUserStr = dynamic_cast<TestStrategy*>(m_ptrUserStrategy.get());
	TestStrategy* pSlaveStr = dynamic_cast<TestStrategy*>(m_ptrNreStrategy.get());


	SDPA_LOG_DEBUG("Slave: send  WorkerRegistrationEvent to "<<m_ptrAgg->name());
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromNre, m_ptrAgg->name()));
	m_ptrAgg->daemon_stage()->send(pEvtWorkerReg);
	pSlaveStr->WaitForEvent<sdpa::events::WorkerRegistrationAckEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: request configuration from "<<m_ptrAgg->name());
	ConfigRequestEvent::Ptr pEvtCfgReq( new ConfigRequestEvent(strFromNre, m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtCfgReq);

	// wait for a configuration reply event
	pSlaveStr->WaitForEvent<sdpa::events::ConfigReplyEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: send LS to "<<m_ptrAgg->name());
	LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromNre, m_ptrAgg->name()));
	m_ptrAgg->daemon_stage()->send(pEvtLS);

	for(int k=0; k<NITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<m_ptrOrch->name());
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUser, m_ptrOrch->name(), "", m_strWorkflow));
		m_ptrOrch->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		// the slave posts a job request
		SDPA_LOG_DEBUG("Slave: post new job request to "<<m_ptrAgg->name());
		RequestJobEvent::Ptr pEvtReq( new RequestJobEvent(strFromNre, m_ptrAgg->name()) );
		m_ptrAgg->daemon_stage()->send(pEvtReq);

		sdpa::job_id_t job_id_slave;
		SubmitJobEvent::Ptr pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
		while( pErrorEvt.get() )
		{
			if(pErrorEvt->error_code() == ErrorEvent::SDPA_ENOJOBAVAIL)
			{
				os.str("");
				os<<"No job available! Try again ...";
				SDPA_LOG_DEBUG(os.str());

				sleep(1);
				//Post new reqest and wait
				RequestJobEvent::Ptr pEvtReqNew( new RequestJobEvent(strFromNre, m_ptrAgg->name()) );
				m_ptrAgg->daemon_stage()->send(pEvtReqNew);
				// wait the master to reply to the job request
				pErrorEvt.reset();
				pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
			}
		}

		job_id_slave = pSubmitJobEvent->job_id();

		// send a SubmitJobAckEvent to master
		// the master should acknowledge the job then
		SDPA_LOG_DEBUG("Slave: send SubmitJobAckEvent to "<<m_ptrAgg->name());
		SubmitJobAckEvent::Ptr pSubmitJobAck( new SubmitJobAckEvent(strFromNre, m_ptrAgg->name(), job_id_slave) );
		m_ptrAgg->daemon_stage()->send(pSubmitJobAck);

		// the slave computes the job ........

		// Now, the user sends a CancelJob  message
		CancelJobEvent::Ptr pCancelJobEvt( new CancelJobEvent(strFromUser, m_ptrOrch->name(), job_id_user) );
		m_ptrOrch->daemon_stage()->send(pCancelJobEvt);

		// the worker expects a CancelJobEvent ...
		CancelJobEvent::Ptr pCancelEvt = pSlaveStr->WaitForEvent<sdpa::events::CancelJobEvent>(pErrorEvt);

		// the worker cancells the job
		SDPA_LOG_DEBUG("SLAVE: Canceled the job "<<pCancelEvt->job_id()<<"!Sending ack to "<<pCancelEvt->from());

		// ... and replies with a CancelJobAckEvent
		CancelJobAckEvent::Ptr pCancelJobAckEvt(new CancelJobAckEvent(pCancelEvt->to(), pCancelEvt->from(), pCancelEvt->job_id()));
		m_ptrAgg->daemon_stage()->send(pCancelJobAckEvt);

		// the user expects now a CancelJobAckEvent
		CancelJobAckEvent::Ptr pCancelAckEvt = pUserStr->WaitForEvent<sdpa::events::CancelJobAckEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("USER: The job "<<pCancelAckEvt->job_id()<<" has been successfully cancelled!");
	}

	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);

	// you can leave now
	SDPA_LOG_DEBUG("Slave: Finished!");
}


void D2DRealGwesTest::testDaemonFSM_JobCancelled_from_Pending()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobCancelled_from_Pending******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string strFromUser(sdpa::daemon::USER);
	string strFromNre(sdpa::daemon::NRE);


	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	TestStrategy* pUserStr = dynamic_cast<TestStrategy*>(m_ptrUserStrategy.get());
	TestStrategy* pSlaveStr = dynamic_cast<TestStrategy*>(m_ptrNreStrategy.get());


	SDPA_LOG_DEBUG("Slave: send  WorkerRegistrationEvent to "<<m_ptrAgg->name());
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(strFromNre, m_ptrAgg->name()));
	m_ptrAgg->daemon_stage()->send(pEvtWorkerReg);
	pSlaveStr->WaitForEvent<sdpa::events::WorkerRegistrationAckEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: request configuration from "<<m_ptrAgg->name());
	ConfigRequestEvent::Ptr pEvtCfgReq( new ConfigRequestEvent(strFromNre, m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtCfgReq);

	// wait for a configuration reply event
	pSlaveStr->WaitForEvent<sdpa::events::ConfigReplyEvent>(pErrorEvt);

	SDPA_LOG_DEBUG("Slave: send LS to "<<m_ptrAgg->name());
	LifeSignEvent::Ptr pEvtLS(new LifeSignEvent(strFromNre, m_ptrAgg->name()));
	m_ptrAgg->daemon_stage()->send(pEvtLS);

	for(int k=0; k<NITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<m_ptrOrch->name());
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUser, m_ptrOrch->name(), "", m_strWorkflow));
		m_ptrOrch->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		// the slave posts a job request
		SDPA_LOG_DEBUG("Slave: post new job request to "<<m_ptrAgg->name());
		RequestJobEvent::Ptr pEvtReq( new RequestJobEvent(strFromNre, m_ptrAgg->name()) );
		m_ptrAgg->daemon_stage()->send(pEvtReq);

		sdpa::job_id_t job_id_slave;
		SubmitJobEvent::Ptr pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
		while( pErrorEvt.get() )
		{
			if(pErrorEvt->error_code() == ErrorEvent::SDPA_ENOJOBAVAIL)
			{
				os.str("");
				os<<"No job available! Try again ...";
				SDPA_LOG_DEBUG(os.str());

				sleep(1);
				//Post new reqest and wait
				RequestJobEvent::Ptr pEvtReqNew( new RequestJobEvent(strFromNre, m_ptrAgg->name()) );
				m_ptrAgg->daemon_stage()->send(pEvtReqNew);
				// wait the master to reply to the job request
				pErrorEvt.reset();
				pSubmitJobEvent = pSlaveStr->WaitForEvent<sdpa::events::SubmitJobEvent>(pErrorEvt);
			}
		}

		job_id_slave = pSubmitJobEvent->job_id();

		// Now, the user sends a CancelJob  message
		CancelJobEvent::Ptr pCancelJobEvt( new CancelJobEvent(strFromUser, m_ptrOrch->name(), job_id_user) );
		m_ptrOrch->daemon_stage()->send(pCancelJobEvt);

		// the worker expects a CancelJobEvent ...
		CancelJobEvent::Ptr pCancelEvt = pSlaveStr->WaitForEvent<sdpa::events::CancelJobEvent>(pErrorEvt);

		// the worker cancells the job
		SDPA_LOG_DEBUG("SLAVE: Canceled the job "<<pCancelEvt->job_id()<<"!Sending ack to "<<pCancelEvt->from());

		// ... and replies with a CancelJobAckEvent
		CancelJobAckEvent::Ptr pCancelJobAckEvt(new CancelJobAckEvent(pCancelEvt->to(), pCancelEvt->from(), pCancelEvt->job_id()));
		m_ptrAgg->daemon_stage()->send(pCancelJobAckEvt);

		// the user expects now a CancelJobAckEvent
		CancelJobAckEvent::Ptr pCancelAckEvt = pUserStr->WaitForEvent<sdpa::events::CancelJobAckEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("USER: The job "<<pCancelAckEvt->job_id()<<" has been successfully cancelled!");
	}

	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);

	// you can leave now
	SDPA_LOG_DEBUG("Slave: Finished!");
}
