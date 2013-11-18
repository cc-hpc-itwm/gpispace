#define BOOST_TEST_MODULE TestStopRestartDrtsPollingCli
#include <boost/test/unit_test.hpp>

#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>

#include "tests_config.hpp"
#include <boost/filesystem/fstream.hpp>

#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>

#include <tests/sdpa/CreateDrtsWorker.hpp>
#include "kvs_setup_fixture.hpp"

namespace bfs=boost::filesystem;
using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

const int NMAXTRIALS = 10;
static int testNb 	 = 0;

namespace po = boost::program_options;

BOOST_GLOBAL_FIXTURE (KVSSetup);

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000) //microseconds
			, m_arrAggMasterInfo(1, MasterInfo("orchestrator_0"))
	{
		LOG(DEBUG, "Fixture's constructor called ...");
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

	boost::thread m_threadClient;
};


void MyFixture::run_client_polling()
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

			boost::this_thread::sleep(boost::posix_time::seconds(1));
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
			boost::this_thread::sleep(boost::posix_time::seconds(1));
			return;
		}
	}

	ptrCli->shutdown_network();
	ptrCli.reset();
}


BOOST_FIXTURE_TEST_SUITE( test_StopRestartAgents, MyFixture );


BOOST_AUTO_TEST_CASE( testStopRestartDrts_RealWE)
{
	LOG( DEBUG, "testStopRestartDrts_RealWE");
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";


	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create_with_start_called("orchestrator_0", addrOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create_with_start_called("agent_0", addrAgent, arrAgentMasterInfo);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_polling, this));

	drts_0->stop();
	if(drts_0_thread.joinable())
		drts_0_thread.join();

	// create new drts
	sdpa::shared_ptr<fhg::core::kernel_t> drts_new( createDRTSWorker("drts_new", "agent_0", "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_new_thread = boost::thread(&fhg::core::kernel_t::run, drts_new);

	if( threadClient.joinable() )
		threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	// and stop!
	drts_new->stop();
	if(drts_new_thread.joinable())
		drts_new_thread.join();

	ptrAgent->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testStopRestartDrts_RealWE terminated!");
}

BOOST_AUTO_TEST_SUITE_END()
