/*
 * =====================================================================================
 *
 *       Filename:  test_Inotify.cpp
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
#define BOOST_TEST_MODULE testInotify

#include <boost/test/unit_test.hpp>
#include "tests_config.hpp"
#include <utils.hpp>

#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>
#include <boost/filesystem/path.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>
#include <boost/thread.hpp>
#include <tests/sdpa/CreateDrtsWorker.hpp>
#include "kvs_setup_fixture.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/inotify.h>

const int NMAXTRIALS=5;
static int testNb = 0;

namespace po = boost::program_options;

using namespace std;

BOOST_GLOBAL_FIXTURE (KVSSetup);

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
			, m_arrAggMasterInfo(1, sdpa::MasterInfo("orchestrator_0"))
	{
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
	sdpa::shared_ptr<fhg::core::kernel_t> create_drts(const std::string& drtsName, const std::string& masterName, const std::string& cpbList );

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

namespace
{
  void touch_file_until_flag_set (bool* keep_on, std::string filename)
  {
    while (*keep_on)
    {
      std::ifstream ifs (filename.c_str());
      if(!ifs.good())
      {
        std::ofstream ofs (filename.c_str());
      }
      boost::this_thread::sleep (boost::posix_time::milliseconds (50));
    }
  }
}

BOOST_AUTO_TEST_CASE( testInotifyExecution )
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/inotify.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent<we::mgmt::layer> agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_0
    ( "drts_0", agent
    , "ATOMIC"
    , TESTS_EXAMPLE_INOTIFY_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  bool keep_on_touching (true);
  boost::thread toucher ( boost::bind ( &touch_file_until_flag_set
                                      , &keep_on_touching
                                      , "inotify_test.txt"
                                      )
                        );

  utils::client::submit_job_and_wait_for_termination
    (workflow, "sdpac", orchestrator);

  keep_on_touching = false;
  if (toucher.joinable())
  {
    toucher.join();
  }
}

BOOST_AUTO_TEST_SUITE_END()
