/*
 * =====================================================================================
 *
 *       Filename:  test_AgentsAndDrts.cpp
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
#define BOOST_TEST_MODULE TestCancelJob
#include "sdpa/daemon/JobFSM.hpp"
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

#include <boost/thread.hpp>

#include "tests_config.hpp"

#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemon/daemonFSM/DaemonFSM.hpp"
#include <seda/Strategy.hpp>
#include <sdpa/client/ClientApi.hpp>

#include <plugins/drts.hpp>
#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <seda/StageRegistry.hpp>

#include <boost/filesystem/path.hpp>

#include <sdpa/engine/DummyWorkflowEngine.hpp>
#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include <sdpa/engine/RealWorkflowEngine.hpp>
#include <boost/thread.hpp>

//plugin
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>


const int NMAXTRIALS=5;
const int MAX_CAP = 100;

namespace po = boost::program_options;

using namespace std;
using namespace sdpa::tests;

#define NO_GUI ""

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("0"); return s; }

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
			, m_pool (0)
	    , m_kvsd (0)
	    , m_serv (0)
	    , m_thrd (0)
			, m_arrAggMasterInfo(1, MasterInfo("orchestrator_0"))
	{ //initialize and start_agent the finite state machine

		FHGLOG_SETUP();

		LOG(DEBUG, "Fixture's constructor called ...");

		m_pool = new fhg::com::io_service_pool(1);
		m_kvsd = new fhg::com::kvs::server::kvsd ("");
		m_serv = new fhg::com::tcp_server ( *m_pool
										  , *m_kvsd
										  , kvs_host ()
										  , kvs_port ()
										  , true
										  );
		m_thrd = new boost::thread (boost::bind ( &fhg::com::io_service_pool::run
												, m_pool
												)
								   );

		m_serv->start();

		LOG(INFO, "kvs daemon is listening on port " << m_serv->port ());

		fhg::com::kvs::global::get_kvs_info().init( kvs_host()
												  , boost::lexical_cast<std::string>(m_serv->port())
												  , boost::posix_time::seconds(10)
												  , 3
												  );

		m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	}

	~MyFixture()
	{
		LOG(DEBUG, "Fixture's destructor called ...");

		sstrOrch.str("");
		sstrAgg.str("");

		m_serv->stop ();
		m_pool->stop ();
		m_thrd->join ();

		delete m_thrd;
		delete m_serv;
		delete m_kvsd;
		delete m_pool;

		seda::StageRegistry::instance().stopAll();
		seda::StageRegistry::instance().clear();
	}

	void run_client();
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

  fhg::com::io_service_pool *m_pool;
	fhg::com::kvs::server::kvsd *m_kvsd;
	fhg::com::tcp_server *m_serv;
	boost::thread *m_thrd;

	sdpa::master_info_list_t m_arrAggMasterInfo;

	std::stringstream sstrOrch;
	std::stringstream sstrAgg;

	boost::thread m_threadClient;

	fhg::core::kernel_t *kernel;
};

void MyFixture::run_client()
{
	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config );
	ptrCli->configure_network( config );


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
      LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

      ptrCli->shutdown_network();
      ptrCli.reset();
      return;
    }
  }

  std::string job_status = ptrCli->queryJob(job_id_user);
  LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

  ptrCli->cancelJob(job_id_user);

  nTrials = 0;
  while( job_status.find("Finished") == std::string::npos &&
       job_status.find("Failed") == std::string::npos &&
       job_status.find("Canceled") == std::string::npos)
  {
    try {
      job_status = ptrCli->queryJob(job_id_user);
      LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

      boost::this_thread::sleep(boost::posix_time::seconds(3));
    }
    catch(const sdpa::client::ClientException& cliExc)
    {
      if(nTrials++ > NMAXTRIALS)
      {
        LOG( DEBUG, "The maximum number of job queries  was exceeded. Giving-up now!");

        ptrCli->shutdown_network();
        ptrCli.reset();
        return;
      }

      boost::this_thread::sleep(boost::posix_time::seconds(3));
    }
  }

  LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

  nTrials = 0;

  try {
      LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
      ptrCli->retrieveResults(job_id_user);
  }
  catch(const sdpa::client::ClientException& cliExc)
  {

    LOG( DEBUG, "The maximum number of trials was exceeded. Giving-up now!");

    ptrCli->shutdown_network();
    ptrCli.reset();
    return;

    boost::this_thread::sleep(boost::posix_time::seconds(3));
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
    return;

    boost::this_thread::sleep(boost::posix_time::seconds(3));
  }

	ptrCli->shutdown_network();
  ptrCli.reset();
}


sdpa::shared_ptr<fhg::core::kernel_t> MyFixture::create_drts(const std::string& drtsName, const std::string& masterName )
{
	sdpa::shared_ptr<fhg::core::kernel_t> kernel(new fhg::core::kernel_t);

	kernel->put("plugin.kvs.host", kvs_host());
	kernel->put("plugin.kvs.port", boost::lexical_cast<std::string>(m_serv->port()));

	kernel->put("plugin.drts.name", drtsName);
	kernel->put("plugin.drts.master", masterName);
	kernel->put("plugin.drts.backlog", "2");
	kernel->put("plugin.drts.request-mode", "false");

	kernel->put("plugin.wfe.library_path", TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	kernel->load_plugin (TESTS_KVS_PLUGIN_PATH);
//	kernel->load_plugin (TESTS_GUI_PLUGIN_PATH);
	kernel->load_plugin (TESTS_WFE_PLUGIN_PATH);
	kernel->load_plugin (TESTS_FVM_FAKE_PLUGIN_PATH);
	kernel->load_plugin (TESTS_DRTS_PLUGIN_PATH);

	return kernel;
}

BOOST_FIXTURE_TEST_SUITE( test_agents, MyFixture )

/*
BOOST_AUTO_TEST_CASE( testCancelJobPath1AgentEmptyWEDrts )
{
	// topology:
	// O
	// |
	// A
	// |
	// drts


	LOG( DEBUG, "testCancelJobPath1AgentEmptyWEDrts");
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create( "orchestrator_0", addrOrch, MAX_CAP );
	ptrOrch->start_agent(false);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAg0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create( "agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAg0->start_agent(false);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( create_drts("drts_0", "agent_0") );
	boost::thread drts_0_thread = boost::thread( &fhg::core::kernel_t::run, drts_0 );

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrAg0->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testCancelJobPath1AgentEmptyWE terminated!");
}
*/

BOOST_AUTO_TEST_CASE( testCancelJobPath1AgentRealWE )
{
	// topology:
	// O
	// |
	// A
	// |
	// drts


	LOG( DEBUG, "testCancelJobPath1AgentRealWE");
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create( "orchestrator_0", addrOrch, MAX_CAP );
	ptrOrch->start_agent(false);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAg0 = sdpa::daemon::AgentFactory<RealWorkflowEngine>::create( "agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAg0->start_agent(false);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( create_drts("drts_0", "agent_0") );
	boost::thread drts_0_thread = boost::thread( &fhg::core::kernel_t::run, drts_0 );

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrAg0->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testCancelJobPath1AgentRealWE terminated!");
}

BOOST_AUTO_TEST_CASE( testCancelJobPath2Drts )
{
	// topology:
	// O
	// |
	// A
	// |
	// A
	// |
	// drts


	LOG( DEBUG, "***** testCancelJobPath2Drts *****"<<std::endl);
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create( "orchestrator_0", addrOrch, MAX_CAP );
	ptrOrch->start_agent(false);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAg0 = sdpa::daemon::AgentFactory<RealWorkflowEngine>::create( "agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAg0->start_agent(false);

	sdpa::master_info_list_t arrAgMaster(1, MasterInfo("agent_0"));
	sdpa::daemon::Agent::ptr_t ptrAg00 = sdpa::daemon::AgentFactory<RealWorkflowEngine>::create( "agent_00", addrAgent, arrAgMaster, MAX_CAP );
	ptrAg00->start_agent(false);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_00( create_drts("drts_00", "agent_00") );
	boost::thread drts_00_thread = boost::thread( &fhg::core::kernel_t::run, drts_00 );

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	drts_00->stop();
	drts_00_thread.join();

	ptrAg00->shutdown();
	ptrAg0->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testOrchestratorNoWe terminated!");
}

BOOST_AUTO_TEST_CASE( testCancelAgentsAndDrtsPath3 )
{

	// topology:
	//    O
	//    |
	//    A
	// 	  |
	// 	  A
	//    |
	//    A
	//    |
	//   drts

	LOG( DEBUG, "***** testCancelAgentsAndDrtsPath3 *****"<<std::endl);
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAg0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAg0->start_agent(false);

	sdpa::master_info_list_t arrAgMaster(1, MasterInfo("agent_0"));
	sdpa::daemon::Agent::ptr_t ptrAg00 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_00", addrAgent, arrAgMaster, MAX_CAP );
	ptrAg00->start_agent(false);

	sdpa::master_info_list_t arrAgM(1, MasterInfo("agent_00"));
	sdpa::daemon::Agent::ptr_t ptrAg000 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_000", addrAgent, arrAgM, MAX_CAP );
	ptrAg000->start_agent(false);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_000( create_drts("drts_000", "agent_000") );
	boost::thread drts_000_thread = boost::thread(&fhg::core::kernel_t::run, drts_000);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	drts_000->stop();
	drts_000_thread.join();

	ptrAg000->shutdown();
	ptrAg00->shutdown();
	ptrAg0->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testCancelAgentsAndDrtsPath3 terminated!");
}

BOOST_AUTO_TEST_CASE( testCancelAgentsNoDrtsTree )
{
	LOG( DEBUG, "***** testCancelAgentsNoDrtsTree *****"<<std::endl);
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAg0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAg0->start_agent(false);

	sdpa::master_info_list_t arrAgMaster(1, MasterInfo("agent_0"));
	sdpa::daemon::Agent::ptr_t ptrAg00 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_00", addrAgent, arrAgMaster, MAX_CAP );
	ptrAg00->start_agent(false);

	sdpa::daemon::Agent::ptr_t ptrAg01 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_01", addrAgent, arrAgMaster, MAX_CAP );
	ptrAg01->start_agent(false);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	ptrAg00->shutdown();
	ptrAg01->shutdown();
	ptrAg0->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testCancelAgentsNoDrtsTree terminated!");
}

/*
BOOST_AUTO_TEST_CASE( testCancelJobPath1AgentNoWE )
{
  LOG( DEBUG, "testCancelJobPath1AgentNoWE");
  //guiUrl
  string guiUrl     = "";
  string workerUrl  = "127.0.0.1:5500";
  string addrOrch   = "127.0.0.1";
  string addrAgent  = "127.0.0.1";

  typedef void OrchWorkflowEngine;

  m_strWorkflow = read_workflow("workflows/stresstest.pnet");

  sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create( "orchestrator_0", addrOrch, MAX_CAP );
  ptrOrch->start_agent(false);

  boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));
  threadClient.join();

  LOG( INFO, "The client thread joined the main thread°!" );
  ptrOrch->shutdown();

  LOG( DEBUG, "The test case testCancelJobPath1AgentNoWE terminated!");
}

BOOST_AUTO_TEST_CASE( testCancelJobPath1AgentEmptyWE )
{
  LOG( DEBUG, "testCancelJobPath1AgentEmptyWE");
  //guiUrl
  string guiUrl     = "";
  string workerUrl  = "127.0.0.1:5500";
  string addrOrch   = "127.0.0.1";
  string addrAgent  = "127.0.0.1";

  typedef void OrchWorkflowEngine;

  m_strWorkflow = read_workflow("workflows/stresstest.pnet");

  sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create( "orchestrator_0", addrOrch, MAX_CAP );
  ptrOrch->start_agent(false);

  boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

  threadClient.join();

  LOG( INFO, "The client thread joined the main thread°!" );

  ptrOrch->shutdown();

  LOG( DEBUG, "The test case testCancelJobPath1AgentEmptyWE terminated!");
}

BOOST_AUTO_TEST_CASE( testCancelJobPath2AgentsNoWE )
{
  LOG( DEBUG, "testCancelJobPath2AgentsNoWE");
  //guiUrl
  string guiUrl     = "";
  string workerUrl  = "127.0.0.1:5500";
  string addrOrch   = "127.0.0.1";
  string addrAgent  = "127.0.0.1";

  typedef void OrchWorkflowEngine;

  m_strWorkflow = read_workflow("workflows/stresstest.pnet");

  sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create( "orchestrator_0", addrOrch, MAX_CAP );
  ptrOrch->start_agent(false);

  sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
  sdpa::daemon::Agent::ptr_t ptrAg0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create( "agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
  ptrAg0->start_agent(false);

  boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

  threadClient.join();

  LOG( INFO, "The client thread joined the main thread°!" );
  boost::this_thread::sleep(boost::posix_time::seconds(20));

  ptrAg0->shutdown();
  ptrOrch->shutdown();

  LOG( DEBUG, "The test case testCancelJobPath2AgentsNoWE terminated!");
}

BOOST_AUTO_TEST_CASE( testCancelJobPath2AgentsEmptyWE )
{
  LOG( DEBUG, "testCancelJobPath2AgentsEmptyWE");
  //guiUrl
  string guiUrl     = "";
  string workerUrl  = "127.0.0.1:5500";
  string addrOrch   = "127.0.0.1";
  string addrAgent  = "127.0.0.1";

  typedef void OrchWorkflowEngine;

  m_strWorkflow = read_workflow("workflows/stresstest.pnet");

    sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create( "orchestrator_0", addrOrch, MAX_CAP );
  ptrOrch->start_agent(false);

  sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
  sdpa::daemon::Agent::ptr_t ptrAg0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create( "agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
  ptrAg0->start_agent(false);

  boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

  threadClient.join();

  LOG( INFO, "The client thread joined the main thread°!" );
  boost::this_thread::sleep(boost::posix_time::seconds(20));

  ptrAg0->shutdown();
  ptrOrch->shutdown();

  LOG( DEBUG, "The test case testCancelJobPath2AgentsEmptyWE terminated!");
}

BOOST_AUTO_TEST_CASE( testCancelJobPath3AgentsNoWE )
{
  LOG( DEBUG, "testCancelJobPath3AgentsNoWE");
  //guiUrl
  string guiUrl     = "";
  string workerUrl  = "127.0.0.1:5500";
  string addrOrch   = "127.0.0.1";
  string addrAgent  = "127.0.0.1";

  typedef void OrchWorkflowEngine;

  m_strWorkflow = read_workflow("workflows/stresstest.pnet");

  sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create( "orchestrator_0", addrOrch, MAX_CAP );
  ptrOrch->start_agent(false);

  sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
  sdpa::daemon::Agent::ptr_t ptrAg0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create( "agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
  ptrAg0->start_agent(false);

  sdpa::master_info_list_t arrAgent1MasterInfo(1, MasterInfo("agent_0"));
  sdpa::daemon::Agent::ptr_t ptrAg1 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create( "agent_1", addrAgent, arrAgent1MasterInfo, MAX_CAP);
  ptrAg1->start_agent(false);

  boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

  threadClient.join();

  LOG( INFO, "The client thread joined the main thread°!" );

  ptrAg1->shutdown();
  ptrAg0->shutdown();
  ptrOrch->shutdown();

  LOG( DEBUG, "The test case testCancelJobPath3AgentsNoWE terminated!");
}

BOOST_AUTO_TEST_CASE( testCancelJobPath3AgentsEmptyWE )
{
  LOG( DEBUG, "testCancelJobPath3AgentsEmptyWE");
  //guiUrl
  string guiUrl     = "";
  string workerUrl  = "127.0.0.1:5500";
  string addrOrch   = "127.0.0.1";
  string addrAgent  = "127.0.0.1";

  typedef void OrchWorkflowEngine;

  m_strWorkflow = read_workflow("workflows/stresstest.pnet");

  sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create( "orchestrator_0", addrOrch, MAX_CAP );
  ptrOrch->start_agent(false);

  sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
  sdpa::daemon::Agent::ptr_t ptrAg0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create( "agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
  ptrAg0->start_agent(false);

  sdpa::master_info_list_t arrAgent1MasterInfo(1, MasterInfo("agent_0"));
  sdpa::daemon::Agent::ptr_t ptrAg1 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create( "agent_1", addrAgent, arrAgent1MasterInfo, MAX_CAP);
  ptrAg1->start_agent(false);

  boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

  threadClient.join();

  LOG( INFO, "The client thread joined the main thread°!" );

  ptrAg1->shutdown();
  ptrAg0->shutdown();
  ptrOrch->shutdown();

  LOG( DEBUG, "The test case testCancelJobPath3AgentsEmptyWE terminated!");
}

BOOST_AUTO_TEST_CASE( testCancelJobPath3AgentsEmptyWEExec )
{
  LOG( DEBUG, "testCancelJobPath3AgentsEmptyWEExec");
  //guiUrl
  string guiUrl     = "";
  string workerUrl  = "127.0.0.1:5500";
  string addrOrch   = "127.0.0.1";
  string addrAgent  = "127.0.0.1";

  typedef void OrchWorkflowEngine;

  m_strWorkflow = read_workflow("workflows/stresstest.pnet");

   sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create( "orchestrator_0", addrOrch, MAX_CAP );
  ptrOrch->start_agent(false);

  sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
  sdpa::daemon::Agent::ptr_t ptrAg0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create( "agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
  ptrAg0->start_agent(false);

  sdpa::master_info_list_t arrAgent1MasterInfo(1, MasterInfo("agent_0"));
  sdpa::daemon::Agent::ptr_t ptrAg1 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create( "agent_1", addrAgent, arrAgent1MasterInfo, MAX_CAP, true);
  ptrAg1->start_agent(false);

  boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

  threadClient.join();

  LOG( INFO, "The client thread joined the main thread°!" );

  ptrAg1->shutdown();
  ptrAg0->shutdown();
  ptrOrch->shutdown();

  LOG( DEBUG, "The test case testCancelJobPath3AgentsEmptyWEExec terminated!");
}
*/

BOOST_AUTO_TEST_SUITE_END()
