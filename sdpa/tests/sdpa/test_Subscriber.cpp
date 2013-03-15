#define BOOST_TEST_MODULE TestSubscriber
#include <sdpa/daemon/JobFSM.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>

#include <sdpa/daemon/Worker.hpp>
#include <sdpa/JobId.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>
#include "boost/serialization/map.hpp"
#include <sdpa/daemon/JobManager.hpp>

#include <boost/serialization/export.hpp>
#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/orchestrator/SchedulerOrch.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

#include <sdpa/client/ClientApi.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/Strategy.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>
#include <boost/thread.hpp>

#include "tests_config.hpp"
#include <boost/filesystem/fstream.hpp>

#include <sdpa/engine/EmptyWorkflowEngine.hpp>

//plugin
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>

namespace bfs=boost::filesystem;
using namespace sdpa::tests;
using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("0"); return s; }

const int NMAXTRIALS = 10;
const int MAX_CAP 	 = 100;
static int testNb 	 = 0;

namespace po = boost::program_options;

#define NO_GUI ""

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000) //microseconds
			, m_pool (0)
	    	, m_kvsd (0)
	    	, m_serv (0)
	    	, m_thrd (0)
			, m_arrAggMasterInfo(1, MasterInfo("orchestrator_0"))
	{ //initialize and start the finite state machine

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

	void run_client_subscriber();
	int subscribe_and_wait ( const std::string &job_id, const sdpa::client::ClientApi::ptr_t &ptrCli );

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

	std::string strBackupOrch;
	std::string strBackupAgent;

	boost::thread m_threadClient;
};


/*returns: 0 job finished, 1 job failed, 2 job cancelled, other value if failures occurred */
int MyFixture::subscribe_and_wait ( const std::string &job_id, const sdpa::client::ClientApi::ptr_t &ptrCli )
{
	typedef boost::posix_time::ptime time_type;
	time_type poll_start = boost::posix_time::microsec_clock::local_time();

	int exit_code(4);

	ptrCli->subscribe(job_id);

	LOG(INFO, "The client successfully subscribed for orchestrator notifications ...");

	std::string job_status;

  	int nTrials = 0;
  	do {

  		LOG(INFO, "start waiting at: " << poll_start);

  		try
  		{
  			if(nTrials<NMAXTRIALS)
  			{
  				boost::this_thread::sleep(boost::posix_time::seconds(1));
  				LOG(INFO, "Re-trying ...");
  			}

			seda::IEvent::Ptr reply( ptrCli->waitForNotification(10000) );

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
				std::cerr<< "got error event: reason := "
							+ err->reason()
							+ " code := "
							+ boost::lexical_cast<std::string>(err->error_code())<<std::endl;

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
	boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
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
	kernel->load_plugin (TESTS_WFE_PLUGIN_PATH);
//	kernel->load_plugin (TESTS_GUI_PLUGIN_PATH);
	kernel->load_plugin (TESTS_DRTS_PLUGIN_PATH);
	kernel->load_plugin (TESTS_FVM_FAKE_PLUGIN_PATH);

	return kernel;
}

BOOST_FIXTURE_TEST_SUITE( test_StopRestartAgents, MyFixture );

/*BOOST_AUTO_TEST_CASE( testAgentsAndDrts_OrchNoWEWait)
{
	LOG( DEBUG, "testAgentsAndDrts_OrchNoWEWait");
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false, strBackupAgent);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( create_drts("drts_0", "agent_0") );
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

	LOG( DEBUG, "The test case testAgentsAndDrts_OrchNoWEWait terminated!");
}
*/

BOOST_AUTO_TEST_CASE( test2Subscribers_OrchNoWEWait)
{
	LOG( DEBUG, "testAgentsAndDrts_OrchNoWEWait");
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";


	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false, strBackupAgent);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( create_drts("drts_0", "agent_0") );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	//boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_subscriber, this));
	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	cav.push_back("--network.timeout=-1");
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli_0 = sdpa::client::ClientApi::create( config, "sdpac_0", "sdpac_0.apps.client.out" );
	ptrCli_0->configure_network( config );

	sdpa::job_id_t job_id_user;

	try {

		//LOG( DEBUG, "Submitting the following test workflow: \n"<<m_strWorkflow);
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

	int rc = subscribe_and_wait(job_id_user, ptrCli_1 );

	try {
			LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
			ptrCli_1->retrieveResults(job_id_user);
			boost::this_thread::sleep(boost::posix_time::seconds(3));
	}
	catch(const sdpa::client::ClientException& cliExc)
	{

		LOG( DEBUG, "The maximum number of trials was exceeded. Giving-up now!");

		ptrCli_1->shutdown_network();
		ptrCli_1.reset();
		return;

		boost::this_thread::sleep(boost::posix_time::seconds(3));
	}

	try {
		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli_1->deleteJob(job_id_user);
		boost::this_thread::sleep(boost::posix_time::seconds(3));
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		LOG( DEBUG, "The maximum number of  trials was exceeded. Giving-up now!");

		ptrCli_1->shutdown_network();
		ptrCli_1.reset();
		return;

		boost::this_thread::sleep(boost::posix_time::seconds(3));
	}

	ptrCli_1->shutdown_network();
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrAgent->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testAgentsAndDrts_OrchNoWEWait terminated!");
}

/*
BOOST_AUTO_TEST_CASE( testAgentsAndDrts_OrchNoWEStopRestart)
{
	LOG( DEBUG, "testAgentsAndDrts_OrchNoWEStopRestart");
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent, arrAgentMasterInfo, MAX_CAP );
	ptrAgent->start_agent(false, strBackupAgent);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( create_drts("drts_0", "agent_0") );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_subscriber, this));

	LOG( DEBUG, "Shutdown the orchestrator");
	ptrOrch->shutdown(strBackupOrch);
	LOG( INFO, "Shutdown the orchestrator. The recovery string is "<<strBackupOrch);

	boost::this_thread::sleep(boost::posix_time::seconds(3));

	// now try to recover the system
	sdpa::daemon::Orchestrator::ptr_t ptrRecOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);

	LOG( INFO, "Re-start the orchestrator. The recovery string is "<<strBackupOrch);
	ptrRecOrch->start_agent(true, strBackupOrch);

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread!" );

	drts_0->stop();
	drts_0_thread.join();

	ptrAgent->shutdown();
	ptrRecOrch->shutdown();

	LOG( DEBUG, "The test case testAgentsAndDrts_OrchNoWEStopRestart terminated!");
}
*/

BOOST_AUTO_TEST_SUITE_END()
