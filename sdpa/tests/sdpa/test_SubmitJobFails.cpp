#define BOOST_TEST_MODULE TestSubmitJobFails
#include <boost/test/unit_test.hpp>
#include <sdpa/daemon/orchestrator/SchedulerOrch.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>
#include "tests_config.hpp"
#include <sdpa/engine/IWorkflowEngine.hpp>
#include <tests/sdpa/CreateDrtsWorker.hpp>
#include "kvs_setup_fixture.hpp"

using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

const int NMAXTRIALS = 3;

namespace po = boost::program_options;
#define NO_GUI ""

BOOST_GLOBAL_FIXTURE (KVSSetup);

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
	{
		LOG(DEBUG, "Fixture's constructor called ...");
		m_strWorkflow = "invalid_workflow";
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
	void run_client_cancel_failed_job();

	int m_nITER;
	int m_sleep_interval ;
    std::string m_strWorkflow;

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

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config );
	ptrCli->configure_network( config );

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
			boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
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
	}

	nTrials = 0;

	try {
			LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
			ptrCli->retrieveResults(job_id_user);
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

		boost::this_thread::sleep(boost::posix_time::seconds(1));	}

	nTrials = 0;

	try {
		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
		boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
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

	ptrCli->shutdown_network();
    ptrCli.reset();
}

void MyFixture::run_client_cancel_failed_job()
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

		LOG( DEBUG, "Submitting an empty workflow ...");
		job_id_user = ptrCli->submitJob("");
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		LOG( ERROR, "Client exception occurred: "<<cliExc.what() );
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
	while( job_status.find("Finished") == std::string::npos &&
		   job_status.find("Failed") == std::string::npos &&
		   job_status.find("Cancelled") == std::string::npos)
	{
		try {
			job_status = ptrCli->queryJob(job_id_user);
			LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);
			boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			LOG( ERROR, "Client exception occurred: "<<cliExc.what() );
			if(nTrials++ > NMAXTRIALS)
			{
				LOG( DEBUG, "The maximum number of trials was exceeded. Giving-up now!");

				ptrCli->shutdown_network();
				ptrCli.reset();
				return;
			}

			boost::this_thread::sleep(boost::posix_time::seconds(1));
		}
	}

	nTrials = 0;

	try {
			LOG( DEBUG, "Cancel the job "<<job_id_user);
			ptrCli->cancelJob(job_id_user);
			boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		LOG( ERROR, "Client exception occurred: "<<cliExc.what() );

		if(nTrials++ > NMAXTRIALS)
		{
			LOG( DEBUG, "The maximum number cancel trials was exceeded. Giving-up now!");

			ptrCli->shutdown_network();
			ptrCli.reset();
			return;
		}

		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}

	nTrials = 0;

	try {
		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
		boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		LOG( ERROR, "Client exception occurred: "<<cliExc.what() );
		if(nTrials++ > NMAXTRIALS)
		{
			LOG( DEBUG, "The maximum number of trials was exceeded. Giving-up now!");

			ptrCli->shutdown_network();
			ptrCli.reset();
			return;
		}

		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}

	ptrCli->shutdown_network();
    ptrCli.reset();
}

BOOST_FIXTURE_TEST_SUITE( test_StopRestartAgents, MyFixture );

BOOST_AUTO_TEST_CASE( testSubmitJobFailure1 )
{
	LOG( INFO, "***** testSubmitJobFailure1 *****"<<std::endl);

	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";

	LOG( INFO, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create("orchestrator_0", addrOrch, 10);
	ptrOrch->start_agent(true);

	sdpa::master_info_list_t arrAggMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgg = sdpa::daemon::AgentFactory<we::mgmt::layer>::create("agent_0", addrAgg, arrAggMasterInfo, 100 );
	ptrAgg->start_agent(true);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	m_threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));
	//boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));

	m_threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case testSubmitJobFailure terminated!" );
}

BOOST_AUTO_TEST_CASE( testSubmitJobFailure2 )
{
	LOG( INFO, "***** testSubmitJobFailure2 *****"<<std::endl);

	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";

	LOG( INFO, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::Orchestrator::create("orchestrator_0", addrOrch, 10);
	ptrOrch->start_agent(true);

	sdpa::master_info_list_t arrAggMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgg = sdpa::daemon::AgentFactory<we::mgmt::layer>::create("agent_0", addrAgg, arrAggMasterInfo, 100 );
	ptrAgg->start_agent(true);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", "agent_0", "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	m_threadClient = boost::thread(boost::bind(&MyFixture::run_client_cancel_failed_job, this));
	//boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));

	m_threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case testSubmitJobFailure terminated!" );
}

BOOST_AUTO_TEST_SUITE_END()
