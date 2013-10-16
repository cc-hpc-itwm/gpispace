/*
 * =====================================================================================
 *
 *       Filename:  test_Coallocation.cpp
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
#define BOOST_TEST_MODULE testCoallocation
#include <boost/test/unit_test.hpp>
#include "tests_config.hpp"
#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>
#include <tests/sdpa/CreateDrtsWorker.hpp>
#include "kvs_setup_fixture.hpp"

const int NMAXTRIALS=5;
const int NWORKERS=5;
const int MAX_CAP = 100;
static int testNb = 0;

namespace po = boost::program_options;

using namespace std;

#define NO_GUI ""

BOOST_GLOBAL_FIXTURE (KVSSetup);

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
			, m_arrAggMasterInfo(1, sdpa::MasterInfo("orchestrator_0"))
	{
		LOG(DEBUG, "Fixture's constructor called ...");
		m_strWorkflow = read_workflow("workflows/coallocation_test.pnet");
	}

	~MyFixture()
	{
		LOG(DEBUG, "Fixture's destructor called ...");

		sstrOrch.str("");
		sstrAgg.str("");

		seda::StageRegistry::instance().stopAll();
		seda::StageRegistry::instance().clear();
	}

	void run_client();

	string read_workflow(string strFileName)
	{
		ifstream f(strFileName.c_str());
		ostringstream os;
		os.str("");

		BOOST_REQUIRE (f.is_open());

		char c;
		while (f.get(c)) os<<c;
		f.close();

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
		}
		catch(const sdpa::client::ClientException& cliExc)
		{

			LOG( DEBUG, "The maximum number of trials was exceeded. Giving-up now!");

			ptrCli->shutdown_network();
			ptrCli.reset();
			return;

			boost::this_thread::sleep(boost::posix_time::seconds(1));
		}

		nTrials = 0;

		try {
			LOG( DEBUG, "User: delete the job "<<job_id_user);
			ptrCli->deleteJob(job_id_user);
			//boost::this_thread::sleep(boost::posix_time::seconds(3));
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

/*
BOOST_AUTO_TEST_CASE( testCoallocationWorkflow )
{
	LOG( INFO, "***** Test capabilities *****"<<std::endl);
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/coallocation_test.pnet");

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	sdpa::master_info_list_t arrAgentMasterInfo(1, sdpa::MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false);

	boost::thread drts_thread[NWORKERS];
	sdpa::shared_ptr<fhg::core::kernel_t> drts[NWORKERS];

	ostringstream oss; int i;
	for(i=0;i<2;i++)
	{
		oss<<"drts_"<<i;
		drts[i] = createDRTSWorker(oss.str(), "agent_0", "A", TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH, kvs_host(), kvs_port());
		drts_thread[i] = boost::thread( &fhg::core::kernel_t::run, drts[i] );
		oss.str("");
	}

	for(i=2;i<NWORKERS;i++)
	{
		oss<<"drts_"<<i;
		drts[i] = createDRTSWorker(oss.str(), "agent_0", "B", TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH, kvs_host(), kvs_port());
		drts_thread[i] = boost::thread( &fhg::core::kernel_t::run, drts[i] );
		oss.str("");
	}

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

	if(threadClient.joinable())
		threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	for(i=2;i<NWORKERS;i++)
	{
		drts[i]->stop();
		if(drts_thread[i].joinable())
			drts_thread[i].join();
	}

	ptrAgent->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case Test1 terminated!");
}
*/

BOOST_AUTO_TEST_CASE( TestStopRestartCoalloc )
{
	LOG( INFO, "***** Test stop/restart *****"<<std::endl);
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/coallocation_test.pnet");

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	sdpa::master_info_list_t arrAgentMasterInfo(1, sdpa::MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false);

	boost::thread drts_thread[NWORKERS];
	sdpa::shared_ptr<fhg::core::kernel_t> drts[NWORKERS];

	ostringstream oss; int i;
	for(i=0;i<2;i++)
	{
		oss<<"drts_"<<i;
		drts[i] = createDRTSWorker(oss.str(), "agent_0", "A", TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH, kvs_host(), kvs_port());
		drts_thread[i] = boost::thread( &fhg::core::kernel_t::run, drts[i] );
		oss.str("");
	}

	for(i=2;i<NWORKERS;i++)
	{
		oss<<"drts_"<<i;
		drts[i] = createDRTSWorker(oss.str(), "agent_0", "B", TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH, kvs_host(), kvs_port());
		drts_thread[i] = boost::thread( &fhg::core::kernel_t::run, drts[i] );
		oss.str("");
	}

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

	// stop the last worker
	i = 4; //NWORKERS-1;
	drts[i]->stop();
	if(drts_thread[i].joinable())
		drts_thread[i].join();

	LOG( INFO, "Stopping now the last worker ...");
	sleep(5);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_new(createDRTSWorker("drts_new", "agent_0", "B", TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH, kvs_host(), kvs_port()));
	boost::thread drts_thread_new = boost::thread( &fhg::core::kernel_t::run, drts_new );

	if(threadClient.joinable())
		threadClient.join();

	LOG( INFO, "The client thread joined the main thread!" );

	for(i=0;i<NWORKERS;i++)
	{
		drts[i]->stop();
		if(drts_thread[i].joinable())
			drts_thread[i].join();
	}

	drts_new->stop();
	if(drts_thread_new.joinable())
		drts_thread_new.join();

	ptrAgent->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case Test1 terminated!");
}


BOOST_AUTO_TEST_SUITE_END()
