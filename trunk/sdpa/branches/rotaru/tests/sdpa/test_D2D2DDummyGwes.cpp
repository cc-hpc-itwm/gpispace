/*
 * =====================================================================================
 *
 *       Filename:  test_D2D2DDummyGwes.hpp
 *
 *    Description:  test 3 generic daemons, each with a dummy gwes
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
#include "test_D2D2DDummyGwes.hpp"
#include <DaemonTestUtil.h>

CPPUNIT_TEST_SUITE_REGISTRATION( D2D2DDummyGwesTest );

D2D2DDummyGwesTest::D2D2DDummyGwesTest() :
	SDPA_INIT_LOGGER("sdpa.tests.D2D2DDummyGwesTest"),
	m_nITER(1),
	m_sleep_interval(10000)
{
}

D2D2DDummyGwesTest::~D2D2DDummyGwesTest()
{}


string D2D2DDummyGwesTest::read_workflow(string strFileName)
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

void D2D2DDummyGwesTest::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	m_ptrSdpa2GwesOrch = new DummyGwes();
	m_ptrSdpa2GwesAgg  = new DummyGwes();

	m_ptrUserStrategy = seda::Strategy::Ptr( new UserStrategy("User") );
	m_ptrToUserStage  = seda::Stage::Ptr(new seda::Stage("to_master_stage", m_ptrUserStrategy) );

	m_ptrOrch = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::ORCHESTRATOR,
												m_ptrToUserStage.get(),
												NULL,
												m_ptrSdpa2GwesOrch) ); // Orchestrator gwes instance
	DaemonFSM::create_daemon_stage(m_ptrOrch);


	m_ptrAgg = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::AGGREGATOR,
								m_ptrOrch->daemon_stage(),
								NULL,
								m_ptrSdpa2GwesAgg) ); // Aggregator gwes instance
	DaemonFSM::create_daemon_stage(m_ptrAgg);


	m_strWorkflow = read_workflow("workflows/masterworkflow-sdpa-test.gwdl");
	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);
}

void D2D2DDummyGwesTest::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	seda::StageRegistry::instance().lookup(m_ptrToUserStage->name())->stop();

	m_ptrNRE->stop();
	m_ptrAgg->stop();
	m_ptrOrch->stop();

	seda::StageRegistry::instance().clear();

	//m_ptrOrch.reset();
	delete m_ptrSdpa2GwesOrch;
	delete m_ptrSdpa2GwesAgg;
}

void D2D2DDummyGwesTest::testDaemonFSM_JobFinished()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobFinished******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string answerStrategy = "finished";
	string strFromUser(sdpa::daemon::USER);
	m_ptrNRE = DaemonFSM::ptr_t (new NreDaemon( sdpa::daemon::NRE, m_ptrAgg->daemon_stage(),
												NULL, NULL, answerStrategy )); // No gwes
	DaemonFSM::create_daemon_stage(m_ptrNRE);

	m_ptrAgg->set_to_slave_stage(m_ptrNRE->daemon_stage());
	m_ptrOrch->set_to_slave_stage(m_ptrAgg->daemon_stage());

	DaemonFSM::start(m_ptrOrch);
	DaemonFSM::start(m_ptrAgg);
	DaemonFSM::start(m_ptrNRE);

	//create output stage
	seda::StageRegistry::instance().insert(m_ptrToUserStage);
	m_ptrToUserStage->start();

	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	UserStrategy* pUserStr = dynamic_cast<UserStrategy*>(m_ptrUserStrategy.get());

	for(int k=0; k<m_nITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<m_ptrOrch->name());
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUser, m_ptrOrch->name(), "", m_strWorkflow, ""));
		m_ptrOrch->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

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
			usleep(m_sleep_interval);
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

	InterruptEvent::Ptr pEvtIntNRE( new InterruptEvent(m_ptrNRE->name(), m_ptrNRE->name() ));
	m_ptrNRE->daemon_stage()->send(pEvtIntNRE);

	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);

	// you can leave now
	SDPA_LOG_DEBUG("User finished!");
}

void D2D2DDummyGwesTest::testDaemonFSM_JobFailed()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobFailed******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string answerStrategy = "failed";
	string strFromUser(sdpa::daemon::USER);
	m_ptrNRE = DaemonFSM::ptr_t (new NreDaemon( sdpa::daemon::NRE,
										m_ptrAgg->daemon_stage(),
										NULL,
										NULL,
										answerStrategy) ); // No gwes
	DaemonFSM::create_daemon_stage(m_ptrNRE);

	m_ptrAgg->set_to_slave_stage(m_ptrNRE->daemon_stage());
	m_ptrOrch->set_to_slave_stage(m_ptrAgg->daemon_stage());

	DaemonFSM::start(m_ptrOrch);
	DaemonFSM::start(m_ptrAgg);
	DaemonFSM::start(m_ptrNRE);

	//create output stage
	seda::StageRegistry::instance().insert(m_ptrToUserStage);
	m_ptrToUserStage->start();

	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	UserStrategy* pUserStr = dynamic_cast<UserStrategy*>(m_ptrUserStrategy.get());

	for(int k=0; k<m_nITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<m_ptrOrch->name());
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUser, m_ptrOrch->name(), "", m_strWorkflow, ""));
		m_ptrOrch->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

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
			usleep(m_sleep_interval);
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

	InterruptEvent::Ptr pEvtIntNRE( new InterruptEvent(m_ptrNRE->name(), m_ptrNRE->name() ));
	m_ptrNRE->daemon_stage()->send(pEvtIntNRE);

	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);

	// you can leave now
	SDPA_LOG_DEBUG("User finished!");
}

void D2D2DDummyGwesTest::testDaemonFSM_JobCancelled()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobCancelled******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string answerStrategy = "cancelled";
	string strFromUser(sdpa::daemon::USER);
	m_ptrNRE = DaemonFSM::ptr_t (new NreDaemon( sdpa::daemon::NRE,
										m_ptrAgg->daemon_stage(),
										NULL,
										NULL,
										answerStrategy) ); // No gwes
	DaemonFSM::create_daemon_stage(m_ptrNRE);

	m_ptrAgg->set_to_slave_stage(m_ptrNRE->daemon_stage());
	m_ptrOrch->set_to_slave_stage(m_ptrAgg->daemon_stage());

	DaemonFSM::start(m_ptrOrch);
	DaemonFSM::start(m_ptrAgg);
	DaemonFSM::start(m_ptrNRE);

	//create output stage
	seda::StageRegistry::instance().insert(m_ptrToUserStage);
	m_ptrToUserStage->start();

	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	UserStrategy* pUserStr = dynamic_cast<UserStrategy*>(m_ptrUserStrategy.get());

	for(int k=0; k<m_nITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<m_ptrOrch->name());
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUser, m_ptrOrch->name(), "", m_strWorkflow, ""));
		m_ptrOrch->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		sleep(1);
		// Now, the user sends a CancelJob  message
		CancelJobEvent::Ptr pCancelJobEvt( new CancelJobEvent(strFromUser, m_ptrOrch->name(), job_id_user) );
		m_ptrOrch->daemon_stage()->send(pCancelJobEvt);

		// the user expects now a CancelJobAckEvent
		CancelJobAckEvent::Ptr pCancelAckEvt = pUserStr->WaitForEvent<sdpa::events::CancelJobAckEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("USER: The job "<<pCancelAckEvt->job_id()<<" has been successfully cancelled!");

		// waits until the job was explicitely cancelled
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
			usleep(m_sleep_interval);
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

	InterruptEvent::Ptr pEvtIntNRE( new InterruptEvent(m_ptrNRE->name(), m_ptrNRE->name() ));
	m_ptrNRE->daemon_stage()->send(pEvtIntNRE);

	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);

	// you can leave now
	SDPA_LOG_DEBUG("User finished!");
}

void D2D2DDummyGwesTest::testDaemonFSM_JobCancelled_from_Pending()
{
	ostringstream os;
	os<<std::endl<<"************************************testDaemonFSM_JobCancelled_from_Pending******************************************"<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	string answerStrategy = "cancelled";
	string strFromUser(sdpa::daemon::USER);
	m_ptrNRE = DaemonFSM::ptr_t (new NreDaemon( sdpa::daemon::NRE,
										m_ptrAgg->daemon_stage(),
										NULL,
										NULL,
										answerStrategy) ); // No gwes
	DaemonFSM::create_daemon_stage(m_ptrNRE);

	m_ptrAgg->set_to_slave_stage(m_ptrNRE->daemon_stage());
	m_ptrOrch->set_to_slave_stage(m_ptrAgg->daemon_stage());

	DaemonFSM::start(m_ptrOrch);
	DaemonFSM::start(m_ptrAgg);
	DaemonFSM::start(m_ptrNRE);

	//create output stage
	seda::StageRegistry::instance().insert(m_ptrToUserStage);
	m_ptrToUserStage->start();

	//ring strDaemon = strDaemon;
	sdpa::events::ErrorEvent::Ptr pErrorEvt;

	UserStrategy* pUserStr = dynamic_cast<UserStrategy*>(m_ptrUserStrategy.get());

	for(int k=0; k<m_nITER;k++)
	{
		// the user submits a job
		// no Jobid set!
		SDPA_LOG_DEBUG("User: submit new job to "<<m_ptrOrch->name());
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(strFromUser, m_ptrOrch->name(), "", m_strWorkflow, ""));
		m_ptrOrch->daemon_stage()->send(pEvtSubmitJob);

		// the user waits for an acknowledgment
		sdpa::job_id_t job_id_user = pUserStr->WaitForEvent<sdpa::events::SubmitJobAckEvent>(pErrorEvt)->job_id();

		sleep(1);
		// Now, the user sends a CancelJob  message
		CancelJobEvent::Ptr pCancelJobEvt( new CancelJobEvent(strFromUser, m_ptrOrch->name(), job_id_user) );
		m_ptrOrch->daemon_stage()->send(pCancelJobEvt);

		// the user expects now a CancelJobAckEvent
		CancelJobAckEvent::Ptr pCancelAckEvt = pUserStr->WaitForEvent<sdpa::events::CancelJobAckEvent>(pErrorEvt);
		SDPA_LOG_DEBUG("USER: The job "<<pCancelAckEvt->job_id()<<" has been successfully cancelled!");

		// waits until the job was explicitely cancelled
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
			usleep(m_sleep_interval);
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

	InterruptEvent::Ptr pEvtIntNRE( new InterruptEvent(m_ptrNRE->name(), m_ptrNRE->name() ));
	m_ptrNRE->daemon_stage()->send(pEvtIntNRE);

	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);

	// you can leave now
	SDPA_LOG_DEBUG("User finished!");
}
