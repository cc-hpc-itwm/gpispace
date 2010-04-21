/*
 * =====================================================================================
 *
 *       Filename:  test_Components.cpp
 *
 *    Description:  test all components, each with a real gwes, using a real user client
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
#include "test_Components.hpp"
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/aggregator/Aggregator.hpp>
#include <sdpa/daemon/nre/NRE.hpp>
#include <sdpa/daemon/nre/SchedulerNRE.hpp>
#include <seda/StageRegistry.hpp>
#include <tests/sdpa/DummyWorkflowEngine.hpp>

namespace po = boost::program_options;

using namespace std;
using namespace sdpa::tests;

#define NO_GUI ""

CPPUNIT_TEST_SUITE_REGISTRATION( TestComponents );

TestComponents::TestComponents() :
	SDPA_INIT_LOGGER("sdpa.tests.TestComponents"),
    m_nITER(10),
    m_sleep_interval(1000000)
{
}

TestComponents::~TestComponents()
{}


string TestComponents::read_workflow(string strFileName)
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

void TestComponents::setUp() { //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
    cav.push_back("--orchestrator=orchestrator_0");
    cav.push_back("--network.location=orchestrator_0:127.0.0.1:5000");
    config.parse_command_line(cav);

	m_ptrCli = sdpa::client::ClientApi::create( config );
	m_ptrCli->configure_network( config );

	seda::Stage::Ptr user_stage = seda::StageRegistry::instance().lookup(m_ptrCli->input_stage());

	m_strWorkflow = read_workflow("workflows/masterworkflow-sdpa-test.gwdl");
				    //read_workflow("workflows/remig.master.gwdl");

	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);
}

void TestComponents::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	m_ptrCli->shutdown_network();
	m_ptrCli.reset();
	seda::StageRegistry::instance().clear();
}

void TestComponents::testComponentsRealGWES()
{
	SDPA_LOG_DEBUG("*****testComponents*****"<<std::endl);

	/*string strAnswer = "finished";
	string noStage = "";
	string strGuiUrl = "";

	//sdpa::shared_ptr<sdpa::nre::worker::ActivityExecutor>
	//	executor(new sdpa::nre::worker::ActivityExecutor("127.0.0.1:8000"));
	// /p/hpc/sdpa/fvm-pc/libexec/remig/*.so
    //executor->loader().load("/path/to/libdummy.so");
    //executor->start();

    // run tests

	//LD_PRELOAD="/p/hpc/sdpa/fvm-pc"/lib/libfvm-pc_fake.so /p/hpc/sdpa/fvm-pc/bin/nre-pcd -c /p/hpc/sdpa/fvm-pc/etc/fvm.cfg.tpl $@


	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::ptr_t ptrOrch = sdpa::daemon::Orchestrator<DummyWorkflowEngine>::create("orchestrator_0", "127.0.0.1:7000", "workflows" );
	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::start(ptrOrch);

	sdpa::daemon::Aggregator<DummyWorkflowEngine>::ptr_t ptrAgg = sdpa::daemon::Aggregator<DummyWorkflowEngine>::create("aggregator_0", "127.0.0.1:7001","orchestrator_0", "127.0.0.1:7000");
	sdpa::daemon::Aggregator<DummyWorkflowEngine>::start(ptrAgg);

	// use external scheduler and real GWES
	sdpa::daemon::NRE<DummyWorkflowEngine>::ptr_t ptrNRE_0 = sdpa::daemon::NRE<DummyWorkflowEngine>::create("NRE_0",  "127.0.0.1:7002","aggregator_0", "127.0.0.1:7001", "127.0.0.1:8000", strGuiUrl, true );
	//sdpa::daemon::NRE<DummyWorkflowEngine>::ptr_t ptrNRE_1 = sdpa::daemon::NRE<DummyWorkflowEngine>::create( "NRE_1",  "127.0.0.1:7003","aggregator_0", "127.0.0.1:7001" );

    try
    {
    	sdpa::daemon::NRE<DummyWorkflowEngine>::start(ptrNRE_0);
    	//sdpa::daemon::NRE<DummyWorkflowEngine>::start(ptrNRE_1);
    }
    catch (const std::exception &ex)
    {
    	LOG(FATAL, "could not start NRE: " << ex.what());
    	LOG(WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

    	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::shutdown(ptrOrch);
    	sdpa::daemon::Aggregator<DummyWorkflowEngine>::shutdown(ptrAgg);
    	sdpa::daemon::NRE<DummyWorkflowEngine>::shutdown(ptrNRE_0);
    	//sdpa::daemon::NRE<DummyWorkflowEngine>::shutdown(ptrNRE_1);

    	return;
    }

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

	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::shutdown(ptrOrch);
	sdpa::daemon::Aggregator<DummyWorkflowEngine>::shutdown(ptrAgg);
	sdpa::daemon::NRE<DummyWorkflowEngine>::shutdown(ptrNRE_0);
	//sdpa::daemon::NRE<DummyWorkflowEngine>::shutdown(ptrNRE_1);

	 //executor->stop();
	*/

    sleep(1);
	SDPA_LOG_DEBUG("Test finished!");
}


void TestComponents::testComponentsDummyGWES()
{
	SDPA_LOG_DEBUG("*****testComponents*****"<<std::endl);
	string strAnswer = "finished";
	string noStage = "";

	bool bUseExtSched  = false;
	string strGuiUrl   = "";

	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::ptr_t ptrOrch = sdpa::daemon::Orchestrator<DummyWorkflowEngine>::create("orchestrator_0", "127.0.0.1:7000", "workflows");
	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::start(ptrOrch);

	sdpa::daemon::Aggregator<DummyWorkflowEngine>::ptr_t ptrAgg = sdpa::daemon::Aggregator<DummyWorkflowEngine>::create("aggregator_0", "127.0.0.1:7001","orchestrator_0", "127.0.0.1:7000");
	sdpa::daemon::Aggregator<DummyWorkflowEngine>::start(ptrAgg);

	// use external scheduler and dummy GWES
	sdpa::daemon::NRE<DummyWorkflowEngine>::ptr_t ptrNRE_0 = sdpa::daemon::NRE<DummyWorkflowEngine>::create("NRE_0",  "127.0.0.1:7002","aggregator_0", "127.0.0.1:7001", "127.0.0.1:8000", strGuiUrl );
	//sdpa::daemon::NRE<DummyWorkflowEngine>::ptr_t ptrNRE_1 = sdpa::daemon::NRE<DummyWorkflowEngine>::create( "NRE_1",  "127.0.0.1:7003","aggregator_0", "127.0.0.1:7001", "127.0.0.1:8001", strGuiUrl );

    try
    {
    	sdpa::daemon::NRE<DummyWorkflowEngine>::start(ptrNRE_0);
    	//sdpa::daemon::NRE<DummyWorkflowEngine>::start(ptrNRE_1);
    }
    catch (const std::exception &ex)
    {
    	LOG(FATAL, "could not start NRE: " << ex.what());
    	LOG(WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

    	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::shutdown(ptrOrch);
    	sdpa::daemon::Aggregator<DummyWorkflowEngine>::shutdown(ptrAgg);
    	sdpa::daemon::NRE<DummyWorkflowEngine>::shutdown(ptrNRE_0);
    	//sdpa::daemon::NRE<DummyWorkflowEngine>::shutdown(ptrNRE_1);

    	return;
    }

	for( int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrCli->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("*****JOB #"<<k<<"******");

		std::string job_status =  m_ptrCli->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrCli->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrCli->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrCli->deleteJob(job_id_user);
	}

	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::shutdown(ptrOrch);
	sdpa::daemon::Aggregator<DummyWorkflowEngine>::shutdown(ptrAgg);
	sdpa::daemon::NRE<DummyWorkflowEngine>::shutdown(ptrNRE_0);
	//sdpa::daemon::NRE<DummyWorkflowEngine>::shutdown(ptrNRE_1);

    sleep(1);
	SDPA_LOG_DEBUG("Test finished!");
}
