/*
 * =====================================================================================
 *
 *       Filename:  test_StopRestartAgentsCoalloc.cpp
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
#define BOOST_TEST_MODULE test_StopRestartAgentsCoalloc
#include <boost/test/unit_test.hpp>
#include "tests_config.hpp"
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>
#include <sdpa/engine/EmptyWorkflowEngine.hpp>
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
			:m_sleep_interval(1000) //microseconds
	{
		LOG(DEBUG, "Fixture's constructor called ...");
	}

	~MyFixture()
	{
		LOG(DEBUG, "Fixture's destructor called ...");

		seda::StageRegistry::instance().stopAll();
		seda::StageRegistry::instance().clear();
		testNb++;
	}

	void run_client(const std::string&, const std::string&);
	int subscribe_and_wait ( const std::string &job_id, const sdpa::client::ClientApi::ptr_t &ptrCli );

	string read_workflow(string strFileName)
	{
		ifstream f(strFileName.c_str());
		ostringstream os;
		os.str("");

		BOOST_REQUIRE (f.is_open());

		char c;
		while (f.get(c))
			os<<c;
		f.close();
		return os.str();
	}

	int m_sleep_interval ;
    std::string m_strWorkflow;

	boost::thread m_threadClient;
};

/*returns: 0 job finished, 1 job failed, 2 job cancelled, other value if failures occurred */
int MyFixture::subscribe_and_wait ( const std::string &job_id, const sdpa::client::ClientApi::ptr_t &ptrCli )
{
	typedef boost::posix_time::ptime time_type;
	time_type poll_start = boost::posix_time::microsec_clock::local_time();

	int exit_code(4);
	std::string job_status;
	bool bSubscribed=false;

  	int nTrials = 0;
  	do
  	{
  		do
		{
			try
			{
				ptrCli->subscribe(job_id);
				bSubscribed = true;
			}
			catch(...)
			{
				bSubscribed = false;
				boost::this_thread::sleep(boost::posix_time::seconds(1));
			}

			if(bSubscribed)
				break;

			nTrials++;
			boost::this_thread::sleep(boost::posix_time::seconds(1));

		}while(nTrials<NMAXTRIALS);

		if(bSubscribed)
		{
			LOG(INFO, "The client successfully subscribed for orchestrator notifications ...");
			nTrials = 0;
		}
		else
		{
			LOG(INFO, "Could not connect to the orchestrator. Giving-up, now!");
		  	return exit_code;
		}


  		LOG(INFO, "start waiting at: " << poll_start);

  		try
  		{
  			if(nTrials<NMAXTRIALS)
  			{
  				boost::this_thread::sleep(boost::posix_time::seconds(1));
  				LOG(INFO, "Re-trying ...");
  			}

			seda::IEvent::Ptr reply( ptrCli->waitForNotification(1000000) );

			// check event type
			if (dynamic_cast<sdpa::events::JobFinishedEvent*>(reply.get()))
			{
				job_status="Finished";
				LOG(WARN, "The job has finished!");
				exit_code = 0;
			}
			else if (dynamic_cast<sdpa::events::JobFailedEvent*>(reply.get()))
			{
				job_status="Failed";
				LOG(WARN, "The job has failed!");
				exit_code = 1;
			}
			else if (dynamic_cast<sdpa::events::CancelJobAckEvent*>(reply.get()))
			{
				LOG(WARN, "The job has been canceled!");
				job_status="Cancelled";
				exit_code = 2;
			}
			else if(sdpa::events::ErrorEvent *err = dynamic_cast<sdpa::events::ErrorEvent*>(reply.get()))
			{
				LOG(WARN, "got error event: reason := "
							+ err->reason()
							+ " code := "
							+ boost::lexical_cast<std::string>(err->error_code()));

				// give some time to the orchestrator to come up
				boost::this_thread::sleep(boost::posix_time::seconds(3));

			}
			else
			{
				LOG(WARN, "unexpected reply: " << (reply ? reply->str() : "null"));
			}
		}
		catch (const sdpa::client::Timedout &)
		{
			LOG(INFO, "Timeout expired!");
		}

  	}while(exit_code == 4 && ++nTrials<NMAXTRIALS);

  	std::cout<<"The status of the job "<<job_id<<" is "<<job_status<<std::endl;

  	if( job_status != std::string("Finished") &&
  		job_status != std::string("Failed")   &&
  		job_status != std::string("Cancelled") )
  	{
  		LOG(ERROR, "Unexpected status, leave now ...");
  		return exit_code;
  	}

  	time_type poll_end = boost::posix_time::microsec_clock::local_time();

  	LOG(INFO, "Client stopped waiting at: " << poll_end);
  	LOG(INFO, "Execution time: " << (poll_end - poll_start));
  	return exit_code;
}

void MyFixture::run_client(const std::string& orchName, const std::string& cliName)
{
	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	std::string prefix("--orchestrator=");
	cav.push_back(prefix+orchName);
	cav.push_back("--network.timeout=-1");
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config, cliName, cliName+".apps.client.out" );
	ptrCli->configure_network( config );

	int nTrials = 0;
	sdpa::job_id_t job_id_user;

	try {

		LOG( INFO, "Submitting new workflow ..."); //<<m_strWorkflow);
		job_id_user = ptrCli->submitJob(m_strWorkflow);
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		if(nTrials++ > NMAXTRIALS)
		{
			LOG( WARN, "The maximum number of job submission  trials was exceeded. Giving-up now!");

			ptrCli->shutdown_network();
			ptrCli.reset();
			return;
		}

		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}

	subscribe_and_wait( job_id_user, ptrCli );

	try {
		LOG( INFO, "The client requests to delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		LOG( WARN, "The maximum number of  trials was exceeded. Giving-up now!");

		ptrCli->shutdown_network();
		ptrCli.reset();
		boost::this_thread::sleep(boost::posix_time::seconds(1));
		return;
	}

	ptrCli->shutdown_network();
	ptrCli.reset();
}


BOOST_FIXTURE_TEST_SUITE( test_dtop_restart_coalloc, MyFixture )

BOOST_AUTO_TEST_CASE( TestStopRestartDrtsCoallocCommonCpb )
{
	LOG( INFO, "***** TestStopRestartDrtsCoallocCommonCpb *****"<<std::endl);

	const int NWORKERS=5;

	//guiUrl
	string guiUrl   	= "";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/coallocation_test.pnet");

	ostringstream osstr;
	osstr<<"orchestrator_"<<testNb;
	std::string orchName(osstr.str());

	osstr.str("");
	osstr<<"client_"<<testNb;
	std::string cliName(osstr.str());

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create(orchName, addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	sdpa::master_info_list_t arrAgentMasterInfo(1, sdpa::MasterInfo(orchName));

	osstr.str("");
	osstr<<"agent_"<<testNb;
	std::string agentName(osstr.str());

	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create(agentName, addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false);

	boost::thread drts_thread[NWORKERS];
	sdpa::shared_ptr<fhg::core::kernel_t> drts[NWORKERS];

	ostringstream oss; int i;
	for(i=0;i<2;i++)
	{
		oss<<"drts_"<<testNb<<"_"<<i;
		drts[i] = createDRTSWorker(oss.str(), agentName, "A", TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH, kvs_host(), kvs_port());
		drts_thread[i] = boost::thread( &fhg::core::kernel_t::run, drts[i] );
		oss.str("");
	}

	for(i=2;i<NWORKERS;i++)
	{
		oss<<"drts_"<<testNb<<"_"<<i;
		drts[i] = createDRTSWorker(oss.str(), agentName, "A,B", TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH, kvs_host(), kvs_port());
		drts_thread[i] = boost::thread( &fhg::core::kernel_t::run, drts[i] );
		oss.str("");
	}

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this, orchName, cliName));

	// stop the last worker
	LOG( INFO, "Stopping now the last worker ...");
	drts[NWORKERS-1]->stop();
	if(drts_thread[ NWORKERS-1].joinable())
		drts_thread[ NWORKERS-1].join();

	oss.str("");
	oss<<"drts_"<<testNb<<"_new";
	sdpa::shared_ptr<fhg::core::kernel_t> drts_new(createDRTSWorker(oss.str(), agentName, "A,B", TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH, kvs_host(), kvs_port()));
	boost::thread drts_thread_new = boost::thread( &fhg::core::kernel_t::run, drts_new );

	if(threadClient.joinable())
		threadClient.join();

	LOG( INFO, "The client thread joined the main thread!" );

	for(i=0;i<NWORKERS-1;i++)
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
}

BOOST_AUTO_TEST_SUITE_END()
