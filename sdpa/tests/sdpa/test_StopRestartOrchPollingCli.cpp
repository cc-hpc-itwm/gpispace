#define BOOST_TEST_MODULE TestStopRestartOrchestratorPollingCli
#include <boost/test/unit_test.hpp>

#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>

#include "tests_config.hpp"
#include <boost/filesystem/fstream.hpp>

#include <sdpa/engine/DummyWorkflowEngine.hpp>
#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>

#include <tests/sdpa/CreateDrtsWorker.hpp>
#include "kvs_setup_fixture.hpp"

namespace bfs=boost::filesystem;
using namespace sdpa::tests;
using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

const int NMAXTRIALS = 10;
const int MAX_CAP 	 = 100;
static int testNb 	 = 0;

namespace po = boost::program_options;

#define NO_GUI ""

BOOST_GLOBAL_FIXTURE (KVSSetup);

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000) //microseconds
			, m_arrAggMasterInfo(1, MasterInfo("orchestrator_0"))
	{
		LOG(DEBUG, "Fixture's constructor called ...");
		m_strWorkflow = read_workflow("workflows/transform_file.pnet");
	}

	~MyFixture()
	{
		LOG(DEBUG, "Fixture's destructor called ...");
		seda::StageRegistry::instance().stopAll();
		seda::StageRegistry::instance().clear();
	}

	void run_client_polling();

	string read_workflow(string strFileName)
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

	int m_nITER;
	int m_sleep_interval ;
    std::string m_strWorkflow;

	sdpa::master_info_list_t m_arrAggMasterInfo;

	std::string strBackupOrch;
	std::string strBackupAgent;

	boost::thread m_threadClient;
};

void MyFixture::run_client_polling()
{
	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	cav.push_back("--network.timeout=-1");
	config.parse_command_line(cav);

	std::ostringstream osstr;
	osstr<<"sdpac_"<<testNb++;

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config, osstr.str(), osstr.str()+".apps.client.out" );
	ptrCli->configure_network( config );

	for( int k=0; k<m_nITER; k++ )
	{
		int nTrials = 0;
		sdpa::job_id_t job_id_user;

		try {

			//LOG( DEBUG, "Submitting the following test workflow: \n"<<m_strWorkflow);
			job_id_user = ptrCli->submitJob(m_strWorkflow);
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			if(nTrials++ > NMAXTRIALS)
			{
				LOG( ERROR, "The maximum number of job submission  trials was exceeded. Last error was: "<<cliExc.what()<<". Giving-up now!");
				ptrCli->shutdown_network();
				ptrCli.reset();
				return;
			}
		}

		nTrials = 0;
		std::string job_status;
		do{
			try {
				job_status = ptrCli->queryJob(job_id_user);
				LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

				boost::this_thread::sleep(boost::posix_time::seconds(1));
			}
			catch(const sdpa::client::ClientException& cliExc)
			{
				if(nTrials++ > NMAXTRIALS)
				{
					LOG( ERROR, "The maximum number of job queries was exceeded. Last error was: "<<cliExc.what()<<". Giving-up now!");

					ptrCli->shutdown_network();
					ptrCli.reset();
					boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
					return;
				}

				boost::this_thread::sleep(boost::posix_time::seconds(1));
			}
		}while( job_status.find("Finished") == std::string::npos &&
				   job_status.find("Failed") == std::string::npos &&
				   job_status.find("Canceled") == std::string::npos);

		nTrials = 0;

		try {
			LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
			ptrCli->retrieveResults(job_id_user);
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			LOG( ERROR, "An exception occurred when trying to retrieve the results of the job "<<job_id_user);
			ptrCli->shutdown_network();
			ptrCli.reset();
			boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
			return;
		}

		nTrials = 0;

		try {
			LOG( DEBUG, "User: delete the job "<<job_id_user);
			ptrCli->deleteJob(job_id_user);
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			LOG( ERROR, "An exception occurred when trying to delete the results of the job "<<job_id_user);
			ptrCli->shutdown_network();
			ptrCli.reset();
			boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
			return;
		}
	}

	ptrCli->shutdown_network();
    ptrCli.reset();
}

BOOST_FIXTURE_TEST_SUITE( test_StopRestartOrchestrator, MyFixture );

BOOST_AUTO_TEST_CASE( testAgentsAndDrts_Orch2Agents)
{
	LOG( DEBUG, "testAgentsAndDrts_Orch2Agents");
	//guiUrl
	string guiUrl     = "";
	string workerUrl  = "127.0.0.1:5500";
	string addrOrch   = "127.0.0.1";
	string addrAgent0 = "127.0.0.1";
	string addrAgent1 = "127.0.0.1";

	std::string strBackupAgent0;
    std::string strBackupAgent1;

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");
	//LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgent0MasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent0, arrAgent0MasterInfo, MAX_CAP );
	ptrAgent0->start_agent(false, strBackupAgent0);

	sdpa::master_info_list_t arrAgent1MasterInfo(1, MasterInfo("agent_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent1 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_1", addrAgent1, arrAgent1MasterInfo, MAX_CAP, true );
	ptrAgent1->start_agent(false, strBackupAgent1);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_polling, this));
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	LOG( DEBUG, "Shutdown the orchestrator");
	ptrOrch->shutdown(strBackupOrch);

	boost::this_thread::sleep(boost::posix_time::seconds(1));

	// now try to recover the system
	sdpa::daemon::Orchestrator::ptr_t ptrRecOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);

	LOG( INFO, "Re-start the orchestrator");// The recovery string is "<<strBackupOrch);
	ptrRecOrch->start_agent(false, strBackupOrch);

	if( threadClient.joinable() )
		threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	ptrAgent1->shutdown();
	ptrAgent0->shutdown();
	ptrRecOrch->shutdown();

	LOG( DEBUG, "The test case testAgentsAndDrts_Orch2Agents terminated!");
}

BOOST_AUTO_TEST_CASE( testAgentsAndDrts_OrchNoWE)
{
	LOG( DEBUG, "testAgentsAndDrts_OrchNoWE");
	//guiUrl
	string guiUrl    = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch  = "127.0.0.1";
	string addrAgent = "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false, strBackupAgent);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_polling, this));

	LOG( DEBUG, "Shutdown the orchestrator");
	ptrOrch->shutdown(strBackupOrch);
	LOG( INFO, "Shutdown the orchestrator. The recovery string is "<<strBackupOrch);

	boost::this_thread::sleep(boost::posix_time::seconds(3));

	// now try to recover the system
	sdpa::daemon::Orchestrator::ptr_t ptrRecOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);

	LOG( INFO, "Re-start the orchestrator. The recovery string is "<<strBackupOrch);
	ptrRecOrch->start_agent(false, strBackupOrch);

	if( threadClient.joinable() )
		threadClient.join();

	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrAgent->shutdown();
	ptrRecOrch->shutdown();

	LOG( DEBUG, "The test case testAgentsAndDrts_OrchNoWE terminated!");
}

BOOST_AUTO_TEST_CASE( testAgentsAndDrts_OrchEmptyWE)
{
	LOG( DEBUG, "testAgentsAndDrts_OrchEmptyWE");
	//guiUrl
	string guiUrl    = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch  = "127.0.0.1";
	string addrAgent = "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<EmptyWorkflowEngine>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false, strBackupAgent);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_polling, this));

	LOG( DEBUG, "Shutdown the orchestrator");
	ptrOrch->shutdown(strBackupOrch);
	LOG( INFO, "Shutdown the orchestrator. The recovery string is "<<strBackupOrch);

	boost::this_thread::sleep(boost::posix_time::seconds(3));

	// now try to recover the system
	sdpa::daemon::Orchestrator::ptr_t ptrRecOrch = sdpa::daemon::OrchestratorFactory<EmptyWorkflowEngine>::create("orchestrator_0", addrOrch, MAX_CAP);

	LOG( INFO, "Re-start the orchestrator. The recovery string is "<<strBackupOrch);
	ptrRecOrch->start_agent(false, strBackupOrch);

	if( threadClient.joinable() )
		threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrAgent->shutdown();
	ptrRecOrch->shutdown();

	LOG( DEBUG, "The test case testAgentsAndDrts_OrchEmptyWE terminated!");
}

BOOST_AUTO_TEST_CASE( testAgentsAndDrts_OrchDummyWE)
{
	LOG( DEBUG, "testAgentsAndDrts_OrchDummyWE");
	//guiUrl
	string guiUrl    = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch  = "127.0.0.1";
	string addrAgent = "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<DummyWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false, strBackupAgent);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_polling, this));

	LOG( DEBUG, "Shutdown the orchestrator");
	ptrOrch->shutdown(strBackupOrch);
	LOG( INFO, "Shutdown the orchestrator. The recovery string is "<<strBackupOrch);

	boost::this_thread::sleep(boost::posix_time::seconds(3));

	// now try to recover the system
	sdpa::daemon::Orchestrator::ptr_t ptrRecOrch = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create("orchestrator_0", addrOrch, MAX_CAP);

	LOG( INFO, "Re-start the orchestrator. The recovery string is "<<strBackupOrch);
	ptrRecOrch->start_agent(false, strBackupOrch);

	if( threadClient.joinable() )
		threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrAgent->shutdown();
	ptrRecOrch->shutdown();

	LOG( DEBUG, "The test case testAgentsAndDrts_OrchDummyWE terminated!");
}

BOOST_AUTO_TEST_CASE( testAgentsAndDrts_OrchNoWE_AgentRealWE)
{
	LOG( DEBUG, "testAgentsAndDrts_OrchNoWE_AgentRealWE");
	//guiUrl
	string guiUrl    = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch  = "127.0.0.1";
	string addrAgent = "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<RealWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false, strBackupAgent);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_polling, this));

	LOG( DEBUG, "Shutdown the orchestrator");
	ptrOrch->shutdown(strBackupOrch);
	LOG( INFO, "Shutdown the orchestrator. The recovery string is "<<strBackupOrch);

	boost::this_thread::sleep(boost::posix_time::seconds(3));

	// now try to recover the system
	sdpa::daemon::Orchestrator::ptr_t ptrRecOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);

	LOG( INFO, "Re-start the orchestrator. The recovery string is "<<strBackupOrch);
	ptrRecOrch->start_agent(false, strBackupOrch);

	if( threadClient.joinable() )
		threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrAgent->shutdown();
	ptrRecOrch->shutdown();

	LOG( DEBUG, "The test case testAgentsAndDrts_OrchNoWE_AgentRealWE terminated!");
}

BOOST_AUTO_TEST_SUITE_END()
