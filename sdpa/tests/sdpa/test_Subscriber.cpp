#define BOOST_TEST_MODULE TestSubscriber
#include <boost/test/unit_test.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>
#include "tests_config.hpp"
#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include <tests/sdpa/CreateDrtsWorker.hpp>
#include "kvs_setup_fixture.hpp"

namespace bfs=boost::filesystem;
using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

const int NMAXTRIALS = 20;
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

	void run_client_subscriber();
	int subscribe_and_wait ( const std::string &job_id, const sdpa::client::ClientApi::ptr_t &ptrCli );

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

void MyFixture::run_client_subscriber()
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


		subscribe_and_wait( job_id_user, ptrCli );
		nTrials = 0;

RETRY:
		try {
			LOG( DEBUG, "User: delete the job "<<job_id_user);
			ptrCli->deleteJob(job_id_user);
			boost::this_thread::sleep(boost::posix_time::seconds(3));
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			LOG(WARN, "Client exception occurred!(trial "<<nTrials<<")");
			if(nTrials++>NMAXTRIALS)
			{
				LOG( DEBUG, "The maximum number of  trials was exceeded. Giving-up now!");
				ptrCli->shutdown_network();
				ptrCli.reset();
				return;
			}

			boost::this_thread::sleep(boost::posix_time::seconds(3));
			goto RETRY;
		}
	}

	ptrCli->shutdown_network();
    ptrCli.reset();
}

BOOST_FIXTURE_TEST_SUITE( test_StopRestartAgents, MyFixture );

BOOST_AUTO_TEST_CASE( Test1)
{
	LOG( INFO, "Test1");
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_subscriber, this));

	LOG( DEBUG, "Shutdown the orchestrator");

	boost::this_thread::sleep(boost::posix_time::seconds(3));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrAgent->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case Test1 terminated!");
}

BOOST_AUTO_TEST_CASE( Test2)
{
	LOG( INFO, "Test2");
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	cav.push_back("--network.timeout=-1");
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli_0 = sdpa::client::ClientApi::create( config, "sdpac_0", "sdpac_0.apps.client.out" );
	ptrCli_0->configure_network( config );

	sdpa::job_id_t job_id_user;

	try {

		job_id_user = ptrCli_0->submitJob(m_strWorkflow);
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");
	}

	ptrCli_0->shutdown_network();
	ptrCli_0.reset();

	//Use another client as subscriber
	sdpa::client::ClientApi::ptr_t ptrCli_1 = sdpa::client::ClientApi::create( config, "sdpac_1", "sdpac_1.apps.client.out" );
	ptrCli_1->configure_network( config );

	subscribe_and_wait(job_id_user, ptrCli_1 );

	try {
		LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
		ptrCli_1->retrieveResults(job_id_user);
		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		LOG( DEBUG, "The maximum number of trials was exceeded. Giving-up now!");

		ptrCli_1->shutdown_network();
		ptrCli_1.reset();
		return;
	}

	try {
		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli_1->deleteJob(job_id_user);
		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		LOG( DEBUG, "The maximum number of  trials was exceeded. Giving-up now!");

		ptrCli_1->shutdown_network();
		ptrCli_1.reset();
		return;
	}

	ptrCli_1->shutdown_network();

	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrAgent->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case Test2 terminated!");
}

BOOST_AUTO_TEST_SUITE_END()
