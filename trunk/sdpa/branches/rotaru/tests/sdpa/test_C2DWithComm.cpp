#include "test_C2DWithComm.hpp"
#include <DaemonTestUtil.h>
#include <gwes/GWES.h>

CPPUNIT_TEST_SUITE_REGISTRATION( C2DWithCommTest );

C2DWithCommTest::C2DWithCommTest() :
	SDPA_INIT_LOGGER("sdpa.tests.C2DWithCommTest"),
    m_nITER(1),
    m_sleep_interval(10000)
{
}

C2DWithCommTest::~C2DWithCommTest()
{}


string C2DWithCommTest::read_workflow(string strFileName)
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

void C2DWithCommTest::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	m_ptrUser = sdpa::client::ClientApi::create("empty config", sdpa::daemon::USER);
	m_ptrUser->configure_network("");

	seda::Stage::Ptr user_stage = seda::StageRegistry::instance().lookup(sdpa::daemon::USER);

	m_strWorkflow = read_workflow("workflows/masterworkflow-sdpa-test.gwdl");
	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);
}

void C2DWithCommTest::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	m_ptrUser.reset();

	m_ptrOrch->stop();

	seda::StageRegistry::instance().clear();

	//m_ptrOrch.reset();
}

void C2DWithCommTest::testUserOrchComm()
{
	SDPA_LOG_DEBUG("*****testUserOrchComm*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	m_ptrSdpa2GwesOrch = new DummyGwes();
	m_ptrOrch = DaemonFSM::ptr_t( new TestDaemon( sdpa::daemon::ORCHESTRATOR, m_ptrSdpa2GwesOrch, strAnswer ) );
	DaemonFSM::create_daemon_stage(m_ptrOrch);
	m_ptrOrch->configure_network();
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

void C2DWithCommTest::testUserOrchCommWithGwes()
{
	SDPA_LOG_DEBUG("*****testUserOrchComm*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	m_ptrSdpa2GwesOrch = new gwes::GWES();
	m_ptrOrch = DaemonFSM::ptr_t( new TestDaemon( sdpa::daemon::ORCHESTRATOR, m_ptrSdpa2GwesOrch, strAnswer ) );
	DaemonFSM::create_daemon_stage(m_ptrOrch);
	m_ptrOrch->configure_network();
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
