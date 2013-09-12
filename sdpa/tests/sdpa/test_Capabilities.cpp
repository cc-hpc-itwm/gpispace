/*
 * =====================================================================================
 *
 *       Filename:  test_Capabilities.cpp
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
#define BOOST_TEST_MODULE testCapabilities
#include <boost/test/unit_test.hpp>
#include "tests_config.hpp"
#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>
#include <tests/sdpa/CreateDrtsWorker.hpp>
#include "kvs_setup_fixture.hpp"

const int NMAXTRIALS=5;
const int MAX_CAP = 100;
static int testNb = 0;

namespace po = boost::program_options;

using namespace std;
using namespace sdpa::tests;

#define NO_GUI ""

BOOST_GLOBAL_FIXTURE (KVSSetup);

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
			, m_arrAggMasterInfo(1, sdpa::MasterInfo("orchestrator_0"))
	{ //initialize and start_agent the finite state machine
		LOG(DEBUG, "Fixture's constructor called ...");

		m_strWorkflow = read_workflow("workflows/capabilities.pnet");
	}

	~MyFixture()
	{
		LOG(INFO, "Fixture's destructor called ...");

		sstrOrch.str("");
		sstrAgg.str("");

		seda::StageRegistry::instance().stopAll();
		seda::StageRegistry::instance().clear();
	}

	void run_client();
	sdpa::shared_ptr<fhg::core::kernel_t> create_drts(const std::string& drtsName, const std::string& masterName, const std::string& cpbList, const std::string& strWfePath );

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

	std::stringstream sstrOrch;
	std::stringstream sstrAgg;

	boost::thread m_threadClient;
};

void MyFixture::run_client()
{
	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
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

		std::string job_status = ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		nTrials = 0;
		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			try {
				job_status = ptrCli->queryJob(job_id_user);
				LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

				boost::this_thread::sleep(boost::posix_time::seconds(1));
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

				boost::this_thread::sleep(boost::posix_time::seconds(1));
			}
		}

		nTrials = 0;

		try {
				LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
				ptrCli->retrieveResults(job_id_user);
				boost::this_thread::sleep(boost::posix_time::seconds(1));
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			LOG( DEBUG, "The maximum number of trials was exceeded. Giving-up now!");

			ptrCli->shutdown_network();
			ptrCli.reset();
			return;
		}

		nTrials = 0;

		try {
			LOG( DEBUG, "User: delete the job "<<job_id_user);
			ptrCli->deleteJob(job_id_user);
			boost::this_thread::sleep(boost::posix_time::seconds(1));
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			LOG( DEBUG, "The maximum number of  trials was exceeded. Giving-up now!");

			ptrCli->shutdown_network();
			ptrCli.reset();
			return;
		}
	}

	ptrCli->shutdown_network();
    ptrCli.reset();
}

BOOST_FIXTURE_TEST_SUITE( test_agents, MyFixture )

BOOST_AUTO_TEST_CASE( Test1 )
{
	LOG( INFO, "***** Test1 *****"<<std::endl);
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/capabilities.pnet");

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	sdpa::master_info_list_t arrAgentMasterInfo(1, sdpa::MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false);


	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "A", TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread( &fhg::core::kernel_t::run, drts_0 );

	sdpa::shared_ptr<fhg::core::kernel_t> drts_1( createDRTSWorker("drts_1", "agent_0", "B", TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH,  kvs_host(), kvs_port()) );
	boost::thread drts_1_thread = boost::thread( &fhg::core::kernel_t::run, drts_1 );

	sdpa::shared_ptr<fhg::core::kernel_t> drts_2( createDRTSWorker("drts_2", "agent_0", "A", TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH,  kvs_host(), kvs_port()) );
	boost::thread drts_2_thread = boost::thread( &fhg::core::kernel_t::run, drts_2 );

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

	if(threadClient.joinable())
		threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	if(drts_0_thread.joinable())
		drts_0_thread.join();

	drts_1->stop();
	if(drts_1_thread.joinable())
		drts_1_thread.join();

	drts_2->stop();
	if(drts_2_thread.joinable())
		drts_2_thread.join();

	ptrAgent->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case Test1 terminated!");
}

BOOST_AUTO_TEST_CASE( testCapabilities_NoMandatoryReq )
{
	LOG( DEBUG, "***** Test2 *****"<<std::endl);
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/capabilities_no_mandatory.pnet");

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	sdpa::master_info_list_t arrAgentMasterInfo(1, sdpa::MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create("agent_1", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false);


	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_1", "", TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH,  kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread( &fhg::core::kernel_t::run, drts_0 );

	sdpa::shared_ptr<fhg::core::kernel_t> drts_1( createDRTSWorker("drts_1", "agent_1", "", TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH,  kvs_host(), kvs_port()) );
	boost::thread drts_1_thread = boost::thread( &fhg::core::kernel_t::run, drts_1 );

	sdpa::shared_ptr<fhg::core::kernel_t> drts_2( createDRTSWorker("drts_2", "agent_1", "", TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH,  kvs_host(), kvs_port()) );
	boost::thread drts_2_thread = boost::thread( &fhg::core::kernel_t::run, drts_2 );

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

	if(threadClient.joinable())
		threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	if(drts_0_thread.joinable())
		drts_0_thread.join();

	drts_1->stop();
	if(drts_1_thread.joinable())
		drts_1_thread.join();

	drts_2->stop();
	if(drts_2_thread.joinable())
		drts_2_thread.join();

	ptrAgent->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case Test2 terminated!");
}


BOOST_AUTO_TEST_SUITE_END()
