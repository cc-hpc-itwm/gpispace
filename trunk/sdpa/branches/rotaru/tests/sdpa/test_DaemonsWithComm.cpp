#include "test_DaemonsWithComm.hpp"
#include <DaemonTestUtil.h>
#include <gwes/GWES.h>

CPPUNIT_TEST_SUITE_REGISTRATION( DaemonsWithCommTest );

DaemonsWithCommTest::DaemonsWithCommTest() :
	SDPA_INIT_LOGGER("sdpa.tests.DaemonsWithCommTest"),
    m_nITER(1),
    m_sleep_interval(10000)
{
}

DaemonsWithCommTest::~DaemonsWithCommTest()
{}


string DaemonsWithCommTest::read_workflow(string strFileName)
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

void DaemonsWithCommTest::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	const sdpa::client::config_t config = sdpa::client::ClientApi::config();
	m_ptrUser = sdpa::client::ClientApi::create( config, sdpa::daemon::USER );
	m_ptrUser->configure_network(config);

	seda::Stage::Ptr user_stage = seda::StageRegistry::instance().lookup(m_ptrUser->input_stage());

	m_strWorkflow = read_workflow("workflows/masterworkflow-sdpa-test.gwdl");
	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);
}

void DaemonsWithCommTest::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	m_ptrUser.reset();

	seda::StageRegistry::instance().clear();
}

void DaemonsWithCommTest::testUserOrchCommDummyGwes()
{
	SDPA_LOG_DEBUG("*****testUserOrchCommDummyGwes*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	m_ptrSdpa2GwesOrch = new DummyGwes();
	m_ptrOrch = DaemonFSM::ptr_t( new TestDaemon( sdpa::daemon::ORCHESTRATOR, m_ptrSdpa2GwesOrch, strAnswer ) );
	DaemonFSM::create_daemon_stage(m_ptrOrch);
	m_ptrOrch->configure_network( "127.0.0.1:5000" );
	DaemonFSM::start(m_ptrOrch);


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

	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);


	// you can leave now
	m_ptrOrch->shutdown_network();
	delete m_ptrSdpa2GwesOrch;
	SDPA_LOG_DEBUG("User finished!");
}

void DaemonsWithCommTest::testUserOrchCommRealGwes()
{
	SDPA_LOG_DEBUG("*****testUserOrchCommRealGwes*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	m_ptrSdpa2GwesOrch = new gwes::GWES();
	m_ptrOrch = DaemonFSM::ptr_t( new TestDaemon( sdpa::daemon::ORCHESTRATOR, m_ptrSdpa2GwesOrch, strAnswer ) );
	DaemonFSM::create_daemon_stage(m_ptrOrch);
	m_ptrOrch->configure_network( "127.0.0.1:5000" );
	DaemonFSM::start(m_ptrOrch);


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

	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);

	// you can leave now
	m_ptrOrch->shutdown_network();
	delete m_ptrSdpa2GwesOrch;
	SDPA_LOG_DEBUG("User finished!");
}

void DaemonsWithCommTest::testUserOrchAggCommDummyGwes()
{
	SDPA_LOG_DEBUG("*****testUserOrchAggCommDummyGwes*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	m_ptrSdpa2GwesOrch = new DummyGwes();
	m_ptrOrch = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::ORCHESTRATOR, m_ptrSdpa2GwesOrch ) );
	DaemonFSM::create_daemon_stage(m_ptrOrch);
	m_ptrOrch->configure_network("127.0.0.1:5000");
	DaemonFSM::start(m_ptrOrch);

	m_ptrSdpa2GwesAgg = new DummyGwes();
	m_ptrAgg = DaemonFSM::ptr_t( new TestDaemon( sdpa::daemon::AGGREGATOR, m_ptrSdpa2GwesAgg, strAnswer ) );
	DaemonFSM::create_daemon_stage(m_ptrAgg);
	m_ptrAgg->configure_network("127.0.0.1:5001", sdpa::daemon::ORCHESTRATOR, "127.0.0.1:5000");
	DaemonFSM::start(m_ptrAgg);

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


	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);


	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);


	// you can leave now
	m_ptrOrch->shutdown_network();
	m_ptrAgg->shutdown_network();
	delete m_ptrSdpa2GwesOrch;
	delete m_ptrSdpa2GwesAgg;
	SDPA_LOG_DEBUG("User finished!");
}

void DaemonsWithCommTest::testUserOrchAggCommRealGwes()
{
	SDPA_LOG_DEBUG("*****testUserOrchAggCommRealGwes*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	m_ptrSdpa2GwesOrch = new gwes::GWES();
	m_ptrOrch = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::ORCHESTRATOR, m_ptrSdpa2GwesOrch ) );
	DaemonFSM::create_daemon_stage(m_ptrOrch);
	m_ptrOrch->configure_network("127.0.0.1:5000");
	DaemonFSM::start(m_ptrOrch);

	m_ptrSdpa2GwesAgg = new gwes::GWES();
	m_ptrAgg = DaemonFSM::ptr_t( new TestDaemon( sdpa::daemon::AGGREGATOR, m_ptrSdpa2GwesAgg, strAnswer ) );
	DaemonFSM::create_daemon_stage(m_ptrAgg);
	m_ptrAgg->configure_network("127.0.0.1:5001", sdpa::daemon::ORCHESTRATOR, "127.0.0.1:5000");
	DaemonFSM::start(m_ptrAgg);

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


	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);


	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);


	// you can leave now
	m_ptrOrch->shutdown_network();
	m_ptrAgg->shutdown_network();
	delete m_ptrSdpa2GwesOrch;
	delete m_ptrSdpa2GwesAgg;
	SDPA_LOG_DEBUG("User finished!");
}

void DaemonsWithCommTest::testUserOrchAggNRECommDummyGwes()
{
	SDPA_LOG_DEBUG("*****testUserOrchAggNRECommDummyGwes*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	m_ptrSdpa2GwesOrch = new DummyGwes();
	m_ptrOrch = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::ORCHESTRATOR, m_ptrSdpa2GwesOrch ) );
	DaemonFSM::create_daemon_stage(m_ptrOrch);
	m_ptrOrch->configure_network("127.0.0.1:5000");
	DaemonFSM::start(m_ptrOrch);

	m_ptrSdpa2GwesAgg = new DummyGwes();
	m_ptrAgg = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::AGGREGATOR, m_ptrSdpa2GwesAgg ) );
	DaemonFSM::create_daemon_stage(m_ptrAgg);
	m_ptrAgg->configure_network("127.0.0.1:5001", sdpa::daemon::ORCHESTRATOR, "127.0.0.1:5000");
	DaemonFSM::start(m_ptrAgg);

	sdpa::Sdpa2Gwes* pGwesNRE = new DummyGwes();
	m_ptrNRE = DaemonFSM::ptr_t( new TestDaemon( sdpa::daemon::NRE, pGwesNRE, strAnswer ) );
	DaemonFSM::create_daemon_stage(m_ptrNRE);
	m_ptrNRE->configure_network("127.0.0.1:5002", sdpa::daemon::AGGREGATOR, "127.0.0.1:5001");
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


	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);


	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);


	// you can leave now
	m_ptrOrch->shutdown_network();
	m_ptrOrch->stop();
	m_ptrAgg->shutdown_network();
	m_ptrAgg->stop();
	m_ptrNRE->shutdown_network();
	m_ptrNRE->stop();

	delete m_ptrSdpa2GwesOrch;
	delete m_ptrSdpa2GwesAgg;
	delete pGwesNRE;
	SDPA_LOG_DEBUG("User finished!");
}

void DaemonsWithCommTest::testUserOrchAggNRECommRealGwes()
{
	SDPA_LOG_DEBUG("*****testUserOrchAggNRECommRealGwes*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	m_ptrSdpa2GwesOrch = new gwes::GWES();
	m_ptrOrch = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::ORCHESTRATOR, m_ptrSdpa2GwesOrch ) );
	DaemonFSM::create_daemon_stage(m_ptrOrch);
	m_ptrOrch->configure_network("127.0.0.1:5000");
	DaemonFSM::start(m_ptrOrch);

	m_ptrSdpa2GwesAgg =  new gwes::GWES();
	m_ptrAgg = DaemonFSM::ptr_t( new DaemonFSM( sdpa::daemon::AGGREGATOR, m_ptrSdpa2GwesAgg ) );
	DaemonFSM::create_daemon_stage(m_ptrAgg);
	m_ptrAgg->configure_network("127.0.0.1:5001", sdpa::daemon::ORCHESTRATOR, "127.0.0.1:5000");
	DaemonFSM::start(m_ptrAgg);

	sdpa::Sdpa2Gwes* pGwesNRE =  new gwes::GWES();
	m_ptrNRE = DaemonFSM::ptr_t( new TestDaemon( sdpa::daemon::NRE, pGwesNRE, strAnswer ) );
	DaemonFSM::create_daemon_stage(m_ptrNRE);
	m_ptrNRE->configure_network("127.0.0.1:5002", sdpa::daemon::AGGREGATOR, "127.0.0.1:5001");
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


	InterruptEvent::Ptr pEvtIntAgg( new InterruptEvent(m_ptrAgg->name(), m_ptrAgg->name() ));
	m_ptrAgg->daemon_stage()->send(pEvtIntAgg);


	InterruptEvent::Ptr pEvtIntOrch( new InterruptEvent(m_ptrOrch->name(), m_ptrOrch->name() ));
	m_ptrOrch->daemon_stage()->send(pEvtIntOrch);


	// you can leave now
	m_ptrOrch->shutdown_network();
	m_ptrOrch->stop();
	m_ptrAgg->shutdown_network();
	m_ptrAgg->stop();
	m_ptrNRE->shutdown_network();
	m_ptrNRE->stop();

	delete m_ptrSdpa2GwesOrch;
	delete m_ptrSdpa2GwesAgg;
	delete pGwesNRE;
	SDPA_LOG_DEBUG("User finished!");
}


