#define BOOST_TEST_MODULE TestMultipleMasters
#include <sdpa/daemon/Job.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>

#include <sdpa/daemon/Worker.hpp>
#include <sdpa/JobId.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include <sdpa/daemon/JobManager.hpp>

#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>

#include "tests_config.hpp"
#include <boost/filesystem/path.hpp>

#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include <tests/sdpa/CreateDrtsWorker.hpp>

#include "kvs_setup_fixture.hpp"
BOOST_GLOBAL_FIXTURE (KVSSetup);

using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

const int NMAXTRIALS = 10000;
const int NMAXTHRDS = 3;
const int MAX_CAP = 100;
static int testNb = 0;

namespace po = boost::program_options;
#define NO_GUI ""

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
			, m_arrAggMasterInfo(1, MasterInfo("orchestrator_0"))
	{ //initialize and start the finite state machine

		LOG(DEBUG, "Fixture's constructor called ...");
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

	std::stringstream sstrOrch;
	std::stringstream sstrAgg;

	sdpa::master_info_list_t m_arrAggMasterInfo;
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

BOOST_FIXTURE_TEST_SUITE( testMultipleMastersSuite, MyFixture );

BOOST_AUTO_TEST_CASE( testMultipleMastersEmptyWEPush )
{
	LOG( INFO, "testMultipleMastersEmptyWEPush");

	//string strAppGuiUrl   	= "";
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgg_0 	= "127.0.0.1:7700";
	string addrAgg_1 	= "127.0.0.1:7701";
	string addrAgg_00 	= "127.0.0.1:7702";

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");

	LOG( INFO, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	LOG( INFO, "Create the Agent ...");
	sdpa::daemon::Agent::ptr_t ptrAgent0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgg_0, m_arrAggMasterInfo, 100);
	ptrAgent0->start_agent(false);

	LOG( INFO, "Create the Agent ...");
	sdpa::daemon::Agent::ptr_t ptrAgent1 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_1", addrAgg_1, m_arrAggMasterInfo, 100);
	ptrAgent1->start_agent(false);

	LOG( INFO, "Create the Agent ...");
	sdpa::master_info_list_t arrAgent00MasterInfo;
	sdpa::daemon::Agent::ptr_t ptrAgent00 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_00", addrAgg_00, arrAgent00MasterInfo, 100);
	ptrAgent00->start_agent(false);

	ptrAgent00->addMaster("agent_0");
	ptrAgent00->addMaster("agent_1");

	sdpa::shared_ptr<fhg::core::kernel_t> drts_00( createDRTSWorker("drts_00", "agent_00", "",  TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_00_thread = boost::thread(&fhg::core::kernel_t::run, drts_00);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));
	threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_00->stop();
	if(drts_00_thread.joinable())
		drts_00_thread.join();

	ptrAgent00->shutdown();
	ptrAgent0->shutdown();
	ptrAgent1->shutdown();

	ptrOrch->shutdown();

	LOG( INFO, "The test case testMultipleMastersEmptyWEPush terminated!" );
}

BOOST_AUTO_TEST_SUITE_END()
