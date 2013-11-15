/*
 * =====================================================================================
 *
 *       Filename:  test_Atomic.cpp
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
#define BOOST_TEST_MODULE testAtomic

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include "tests_config.hpp"
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>
#include <tests/sdpa/CreateDrtsWorker.hpp>
#include "kvs_setup_fixture.hpp"

#include <utils.hpp>

const int NMAXTRIALS=5;

namespace po = boost::program_options;

using namespace std;

BOOST_GLOBAL_FIXTURE (KVSSetup);

struct MyFixture
{
	MyFixture()
	{
	}

	~MyFixture()
	{
		seda::StageRegistry::instance().stopAll();
		seda::StageRegistry::instance().clear();
	}
};

void run_client (std::string workflow)
{
	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	config.parse_command_line(cav);

	std::ostringstream osstr;
	osstr<<"sdpac_0";

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config, osstr.str(), osstr.str()+".apps.client.out" );
	ptrCli->configure_network( config );

		int nTrials = 0;
		sdpa::job_id_t job_id_user;

		try {

			LOG( DEBUG, "Submitting the following test workflow: \n"<<workflow);
			job_id_user = ptrCli->submitJob(workflow);
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

		nTrials = 0;
		while(  job_status.find("Finished") == std::string::npos &&
			      job_status.find("Failed") == std::string::npos &&
			      job_status.find("Canceled") == std::string::npos )
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

	ptrCli->shutdown_network();
    ptrCli.reset();
}

BOOST_FIXTURE_TEST_SUITE( test_agents, MyFixture )

BOOST_AUTO_TEST_CASE( testModiFile )
{
	const char* filename = "test.txt";
	FILE * pFile = fopen(filename,"r");

	if(!pFile) // file does not exist, create it
	{
		pFile = fopen(filename,"w");
    fprintf(pFile, "%d\n", 0);
		fclose(pFile);
	}
	else
	{
		int i = 0;
		if (1 == fscanf(pFile, "%d", &i))
		{
		  fclose(pFile);
		  pFile = fopen(filename,"w");
		  ++i;
		  fprintf(pFile, "%d\n", i);
		  fclose(pFile);
		}
		else
		{
		  fclose(pFile);
		  throw std::runtime_error("testModiFile: could not read old state");
		}
	}
}

BOOST_AUTO_TEST_CASE( testAtomicExecution )
{
	LOG( DEBUG, "***** test_Atomic *****"<<std::endl);

  const std::string workflow
    (utils::require_and_read_file ("workflows/atomic.pnet"));


	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

  const std::string atomic_file ("atomic_test.txt");

  int nInitial (0);
  {
    std::ofstream ofs (atomic_file.c_str ());
    ofs << nInitial << std::endl;
  }

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create_with_start_called("orchestrator_0", addrOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, sdpa::MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create_with_start_called("agent_0", addrAgent, arrAgentMasterInfo);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "ATOMIC", TESTS_EXAMPLE_ATOMIC_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread( &fhg::core::kernel_t::run, drts_0 );

	sdpa::shared_ptr<fhg::core::kernel_t> drts_1( createDRTSWorker("drts_1", "agent_0", "A,B", TESTS_EXAMPLE_ATOMIC_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_1_thread = boost::thread( &fhg::core::kernel_t::run, drts_1 );

	boost::thread threadClient = boost::thread(boost::bind(&run_client, workflow));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();
	drts_0->unload_all();

	drts_1->stop();
	drts_1_thread.join();
	drts_1->unload_all();

	ptrAgent->shutdown();
	ptrOrch->shutdown();

	int nCounterVal=0;
	{
		std::ifstream ifs (atomic_file.c_str());
		BOOST_CHECK (ifs.good());
		ifs>>nCounterVal;
	}

	LOG(INFO, "Intial value was "<<nInitial);
	LOG(INFO, "The counter value now is: "<<nCounterVal);

	nCounterVal-=nInitial;

	const int nTasks (boost::lexical_cast<int> (TESTS_N_ATOMIC_TASKS));

	BOOST_CHECK((nCounterVal==2*nTasks));

	LOG( DEBUG, "The test case test_Atomic terminated!");
}

BOOST_AUTO_TEST_SUITE_END()
