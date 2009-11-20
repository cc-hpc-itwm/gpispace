/*
 * =====================================================================================
 *
 *       Filename: test_C2D2D2DDummyGwes.cpp
 *
 *    Description: test 3 generic daemons, each with a dummy gwes, using a real user client
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
#include "test_C2D2D2DDummyGwes.hpp"
#include <DaemonTestUtil.h>


CPPUNIT_TEST_SUITE_REGISTRATION( C2D2D2DDummyGwesTest );

C2D2D2DDummyGwesTest::C2D2D2DDummyGwesTest() :
	SDPA_INIT_LOGGER("sdpa.tests.C2D2D2DDummyGwesTest"),
    m_nITER(1),
    m_sleep_interval(10000)
{
}

C2D2D2DDummyGwesTest::~C2D2D2DDummyGwesTest()
{}


string C2D2D2DDummyGwesTest::read_workflow(string strFileName)
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

void C2D2D2DDummyGwesTest::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	m_ptrSdpa2GwesOrch = new DummyGwes();
	m_ptrSdpa2GwesAgg  = new DummyGwes();

	//const sdpa::client::config_t config;
	const sdpa::client::config_t config = sdpa::client::ClientApi::config();
	/*
	 * const config_t &cfg
                                  ,const std::string &name_prefix="sdpa.apps.client"
                                  ,const std::string &output_stage="sdpa.apps.client.out"
                                  ,const std::string &orchestrator_name="orchestrator"
                                  ,const std::string &client_location="0.0.0.0:0") throw (ClientException)
	 * */
	m_ptrUser = sdpa::client::ClientApi::create( config, sdpa::daemon::USER, sdpa::daemon::ORCHESTRATOR );
	//m_ptrUser = sdpa::client::ClientApi::create( config );

	seda::Stage::Ptr user_stage = seda::StageRegistry::instance().lookup(m_ptrUser->input_stage());

	m_ptrOrch = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::ORCHESTRATOR, m_ptrSdpa2GwesOrch, m_ptrUser->input_stage() )); // Orchestrator gwes instance
	DaemonFSM::create_daemon_stage(m_ptrOrch);

	m_ptrAgg = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::AGGREGATOR, m_ptrSdpa2GwesAgg, sdpa::daemon::ORCHESTRATOR ) ); // Aggregator gwes instance
	DaemonFSM::create_daemon_stage(m_ptrAgg);

	m_strWorkflow = read_workflow("workflows/masterworkflow-sdpa-test.gwdl");
	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);
}

void C2D2D2DDummyGwesTest::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	m_ptrUser.reset();

	m_ptrNRE->stop();
	m_ptrAgg->stop();
	m_ptrOrch->stop();

	seda::StageRegistry::instance().clear();

	//m_ptrOrch.reset();
	delete m_ptrSdpa2GwesOrch;
	delete m_ptrSdpa2GwesAgg;
}

void C2D2D2DDummyGwesTest::testDaemonFSM_JobFinished()
{
	SDPA_LOG_DEBUG("*****testDaemonFSM_JobFinished*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	m_ptrNRE = DaemonFSM::ptr_t (new NreDaemon( sdpa::daemon::NRE, NULL, sdpa::daemon::AGGREGATOR, noStage, strAnswer ) ); // No gwes
	DaemonFSM::create_daemon_stage(m_ptrNRE);

	m_ptrAgg->set_to_slave_stage(m_ptrNRE->daemon_stage());
	m_ptrOrch->set_to_slave_stage(m_ptrAgg->daemon_stage());

	DaemonFSM::start(m_ptrOrch);
	DaemonFSM::start(m_ptrAgg);
	DaemonFSM::start(m_ptrNRE);

	for(int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrUser->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("*****JOB #"<<k<<"******");

		std::string job_status =  m_ptrUser->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrUser->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrUser->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrUser->deleteJob(job_id_user);
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

void C2D2D2DDummyGwesTest::testDaemonFSM_JobFailed()
{
	SDPA_LOG_DEBUG("*****testDaemonFSM_JobFailed*****"<<std::endl);
	string strAnswer = "failed";
	string noStage = "";

	m_ptrNRE = DaemonFSM::ptr_t (new NreDaemon( sdpa::daemon::NRE, NULL, sdpa::daemon::AGGREGATOR, noStage, strAnswer ) ); // No gwes
	DaemonFSM::create_daemon_stage(m_ptrNRE);

	m_ptrAgg->set_to_slave_stage(m_ptrNRE->daemon_stage());
	m_ptrOrch->set_to_slave_stage(m_ptrAgg->daemon_stage());

	DaemonFSM::start(m_ptrOrch);
	DaemonFSM::start(m_ptrAgg);
	DaemonFSM::start(m_ptrNRE);


	for(int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrUser->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("*****JOB #"<<k<<"******");

		std::string job_status =  m_ptrUser->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrUser->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrUser->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrUser->deleteJob(job_id_user);
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


void C2D2D2DDummyGwesTest::testDaemonFSM_JobCancelled()
{
	SDPA_LOG_DEBUG("*****testDaemonFSM_JobCancelled*****"<<std::endl);

	string strAnswer = "cancelled";
	string noStage = "";

	m_ptrNRE = DaemonFSM::ptr_t (new NreDaemon( sdpa::daemon::NRE, NULL, sdpa::daemon::AGGREGATOR, noStage, strAnswer ) ); // No gwes
	DaemonFSM::create_daemon_stage(m_ptrNRE);

	m_ptrAgg->set_to_slave_stage(m_ptrNRE->daemon_stage());
	m_ptrOrch->set_to_slave_stage(m_ptrAgg->daemon_stage());

	DaemonFSM::start(m_ptrOrch);
	DaemonFSM::start(m_ptrAgg);
	DaemonFSM::start(m_ptrNRE);

	for(int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrUser->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("*****JOB #"<<k<<"******");

		std::string job_status =  m_ptrUser->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrUser->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrUser->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrUser->deleteJob(job_id_user);
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



void C2D2D2DDummyGwesTest::testDaemonFSM_JobFinished_WithGwes()
{
	SDPA_LOG_DEBUG("*****testDaemonFSMWithGwes_JobFinished*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	sdpa::Sdpa2Gwes* pGwesNRE = new DummyGwes();

	m_ptrNRE = DaemonFSM::ptr_t( new NreDaemonWithGwes( sdpa::daemon::NRE,
														pGwesNRE, sdpa::daemon::AGGREGATOR,
														noStage,
														strAnswer ) ); // No gwes
	DaemonFSM::create_daemon_stage(m_ptrNRE);

	m_ptrAgg->set_to_slave_stage(m_ptrNRE->daemon_stage());
	m_ptrOrch->set_to_slave_stage(m_ptrAgg->daemon_stage());

	DaemonFSM::start(m_ptrOrch);
	DaemonFSM::start(m_ptrAgg);
	DaemonFSM::start(m_ptrNRE);

	for(int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrUser->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("*****JOB #"<<k<<"******");

		std::string job_status =  m_ptrUser->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrUser->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrUser->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrUser->deleteJob(job_id_user);
	}

	InterruptEvent::Ptr pEvtIntNRE( new InterruptEvent(m_ptrNRE->name(), m_ptrNRE->name() ));
	m_ptrNRE->daemon_stage()->send(pEvtIntNRE);

	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);

	// shutdown the orchestrator
	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);

	// you can leave now
	delete pGwesNRE;
	SDPA_LOG_DEBUG("User finished!");
}


void C2D2D2DDummyGwesTest::testDaemonFSM_JobFailed_WithGwes()
{
	SDPA_LOG_DEBUG("*****testDaemonFSM_JobFailed_WithGwes*****"<<std::endl);
	string strAnswer = "failed";
	string noStage = "";

	sdpa::Sdpa2Gwes* pGwesNRE = new DummyGwes();

	m_ptrNRE = DaemonFSM::ptr_t( new NreDaemonWithGwes( sdpa::daemon::NRE,
														pGwesNRE, sdpa::daemon::AGGREGATOR,
														noStage,
														strAnswer ) ); // No gwes
	DaemonFSM::create_daemon_stage(m_ptrNRE);

	m_ptrAgg->set_to_slave_stage(m_ptrNRE->daemon_stage());
	m_ptrOrch->set_to_slave_stage(m_ptrAgg->daemon_stage());

	DaemonFSM::start(m_ptrOrch);
	DaemonFSM::start(m_ptrAgg);
	DaemonFSM::start(m_ptrNRE);

	for(int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrUser->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("*****JOB #"<<k<<"******");

		std::string job_status =  m_ptrUser->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrUser->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrUser->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrUser->deleteJob(job_id_user);
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


void C2D2D2DDummyGwesTest::testDaemonFSM_JobCancelled_WithGwes()
{
	SDPA_LOG_DEBUG("*****testDaemonFSM_JobCancelled_WithGwes*****"<<std::endl);

	string strAnswer = "cancelled";
	string noStage = "";

	sdpa::Sdpa2Gwes* pGwesNRE = new DummyGwes();

	m_ptrNRE = DaemonFSM::ptr_t( new NreDaemonWithGwes( sdpa::daemon::NRE,
														pGwesNRE, sdpa::daemon::AGGREGATOR,
														noStage,
														strAnswer ) ); // No gwes
	DaemonFSM::create_daemon_stage(m_ptrNRE);

	m_ptrAgg->set_to_slave_stage(m_ptrNRE->daemon_stage());
	m_ptrOrch->set_to_slave_stage(m_ptrAgg->daemon_stage());

	DaemonFSM::start(m_ptrOrch);
	DaemonFSM::start(m_ptrAgg);
	DaemonFSM::start(m_ptrNRE);

	for(int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrUser->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("*****JOB #"<<k<<"******");

		std::string job_status =  m_ptrUser->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrUser->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrUser->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrUser->deleteJob(job_id_user);
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
