#define BOOST_TEST_MODULE TestStopRestartAgentsPollingCli
#include <boost/test/unit_test.hpp>

#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>

#include "tests_config.hpp"
#include <boost/filesystem/fstream.hpp>

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
	{ //initialize and start the finite state machine

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

	sdpa::shared_ptr<fhg::core::kernel_t> create_drts(const std::string& drtsName, const std::string& masterName );

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

			LOG( DEBUG, "Submitting the following test workflow: \n"<<m_strWorkflow);
			job_id_user = ptrCli->submitJob(m_strWorkflow);
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			if(nTrials++ > NMAXTRIALS)
			{
				LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

				ptrCli->shutdown_network();
				ptrCli.reset();
				return;
			}
		}

		LOG( DEBUG, "//////////JOB #"<<k<<"////////////");


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
				LOG(ERROR, "could not query job: " << cliExc.what());
				if(nTrials++ > NMAXTRIALS)
				{
					LOG( DEBUG, "The maximum number of job queries  was exceeded. Giving-up now!");

					ptrCli->shutdown_network();
					ptrCli.reset();
					return;
				}

				boost::this_thread::sleep(boost::posix_time::seconds(3));
			}
		}while( job_status.find("Finished") == std::string::npos &&
				   job_status.find("Failed") == std::string::npos &&
				   job_status.find("Canceled") == std::string::npos);

		nTrials = 0;

		try {
				LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
				ptrCli->retrieveResults(job_id_user);
				//boost::this_thread::sleep(boost::posix_time::seconds(3));
		}
		catch(const sdpa::client::ClientException& cliExc)
		{

			LOG( DEBUG, "The maximum number of trials was exceeded. Giving-up now!");

			ptrCli->shutdown_network();
			ptrCli.reset();
			boost::this_thread::sleep(boost::posix_time::seconds(1));
			return;
		}

		nTrials = 0;

		try {
			LOG( DEBUG, "User: delete the job "<<job_id_user);
			ptrCli->deleteJob(job_id_user);
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			LOG( DEBUG, "The maximum number of  trials was exceeded. Giving-up now!");

			ptrCli->shutdown_network();
			ptrCli.reset();
			boost::this_thread::sleep(boost::posix_time::seconds(1));
			return;
		}
	}

	ptrCli->shutdown_network();
  ptrCli.reset();
}

BOOST_FIXTURE_TEST_SUITE( test_StopRestartAgents, MyFixture );

BOOST_AUTO_TEST_CASE( testStop_2Agents_NoDrts_push_1)
{
	LOG( DEBUG, "testStop_2Agents_NoDrts_push_1");
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent0 	= "127.0.0.1";
	string addrAgent1	= "127.0.0.1";

	std::string strBackupAgent0;
	std::string strBackupAgent1;

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgent0MasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent0, arrAgent0MasterInfo, MAX_CAP );
	ptrAgent0->start_agent(false, strBackupAgent0);

	sdpa::master_info_list_t arrAgent1MasterInfo(1, MasterInfo("agent_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent1 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_1", addrAgent1, arrAgent1MasterInfo, MAX_CAP, true );
	ptrAgent1->start_agent(false, strBackupAgent1);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_polling, this));

	ptrAgent0->shutdown(strBackupAgent0);
	LOG( INFO, "Shutdown agent \"agent_o\". The recovery string is "<<strBackupAgent0);

	boost::this_thread::sleep(boost::posix_time::seconds(1));

	// now try to recover the system
	sdpa::daemon::Agent::ptr_t ptrRecAgent0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent0, arrAgent0MasterInfo, MAX_CAP );

	LOG( INFO, "Re-start \"agent_0\". The recovery string is "<<strBackupAgent0);
	ptrRecAgent0->start_agent(false, strBackupAgent0);

	if( threadClient.joinable() ) threadClient.join();
	LOG( INFO, "The client thread joined the main thread!");

	ptrAgent1->shutdown();
	ptrRecAgent0->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testStop_2Agents_NoDrts_push_1 terminated!");
}


BOOST_AUTO_TEST_CASE( testStop_AgentWithDrts_push)
{
	LOG( DEBUG, "testStop_AgentWithDrts_push");
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";


	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false, strBackupAgent);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_polling, this));

	ptrAgent->shutdown(strBackupAgent);
	LOG( INFO, "Shutdown the agent \"agent_o\". The recovery string is "<<strBackupAgent);

	boost::this_thread::sleep(boost::posix_time::seconds(1));

	// now try to recover the system
	sdpa::daemon::Agent::ptr_t ptrRecAgent = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );

	LOG( INFO, "Re-start the agent. The recovery string is "<<strBackupAgent);
	ptrRecAgent->start_agent(false, strBackupAgent);

	if( threadClient.joinable() )
		threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrRecAgent->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testStop_AgentWithDrts_push terminated!");
}

BOOST_AUTO_TEST_CASE( testStop_AgentNoWE_push)
{
	LOG( DEBUG, "testStop_AgentNoWE_push");
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");
	//LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<void>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false, strBackupAgent);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_polling, this));

	ptrAgent->shutdown(strBackupAgent);
	LOG( INFO, "Shutdown the agent \"agent_o\". The recovery string is "<<strBackupAgent);

	// now try to recover the system
	sdpa::daemon::Agent::ptr_t ptrRecAgent = sdpa::daemon::AgentFactory<void>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );

	LOG( INFO, "Re-start the agent. The recovery string is "<<strBackupAgent);
	ptrRecAgent->start_agent(false, strBackupAgent);

	if( threadClient.joinable() )
		threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrRecAgent->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testStop_AgentNoWE_push terminated!");
}

BOOST_AUTO_TEST_CASE( testStop_AgentEmptyWE_push)
{
	LOG( DEBUG, "testStop_AgentEmptyWE_push");
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";


	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false, strBackupAgent);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0",  "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_polling, this));

	ptrAgent->shutdown(strBackupAgent);
	LOG( INFO, "Shutdown the agent \"agent_o\". The recovery string is "<<strBackupAgent);

	boost::this_thread::sleep(boost::posix_time::seconds(3));

	// now try to recover the system
	sdpa::daemon::Agent::ptr_t ptrRecAgent = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );

	LOG( INFO, "Re-start the agent. The recovery string is "<<strBackupAgent);
	ptrRecAgent->start_agent(false, strBackupAgent);

	if( threadClient.joinable() ) threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrRecAgent->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testStop_AgentEmptyWE_push terminated!");
}

BOOST_AUTO_TEST_CASE( testStop_AgentRealWE_push)
{
	LOG( DEBUG, "testStop_AgentRealWE_push");
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";


	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<RealWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false, strBackupAgent);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0",  "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_polling, this));

	ptrAgent->shutdown(strBackupAgent);
	LOG( INFO, "Shutdown the agent \"agent_o\". The recovery string is "<<strBackupAgent);

	boost::this_thread::sleep(boost::posix_time::seconds(3));

	// now try to recover the system
	sdpa::daemon::Agent::ptr_t ptrRecAgent = sdpa::daemon::AgentFactory<RealWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );

	LOG( INFO, "Re-start the agent. The recovery string is "<<strBackupAgent);
	ptrRecAgent->start_agent(false, strBackupAgent);

	if( threadClient.joinable() ) threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrRecAgent->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testStop_AgentRealWE_push terminated!");
}

BOOST_AUTO_TEST_SUITE_END()
