/*
 * =====================================================================================
 *
 *       Filename:  test_Components.cpp
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
#define BOOST_TEST_MODULE TestOrchestratorEmptyWe
#include "sdpa/daemon/jobFSM/JobFSM.hpp"
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

#include <boost/thread.hpp>

#include "tests_config.hpp"

#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemon/daemonFSM/DaemonFSM.hpp"
#include <seda/Strategy.hpp>
#include <sdpa/client/ClientApi.hpp>

#include <sdpa/daemon/nre/SchedulerNRE.hpp>
#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/aggregator/AggregatorFactory.hpp>
#include <sdpa/daemon/nre/NREFactory.hpp>
#include <seda/StageRegistry.hpp>

#include <boost/filesystem/path.hpp>

#include <sdpa/engine/DummyWorkflowEngine.hpp>
#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include <sdpa/engine/RealWorkflowEngine.hpp>
#include <boost/thread.hpp>

#ifdef USE_REAL_WE
	#include <sdpa/daemon/nre/nre-worker/NreWorkerClient.hpp>
#else
	#include <sdpa/daemon/nre/BasicWorkerClient.hpp>
#endif


const int NMAXTRIALS=5;
const int MAX_CAP = 100;


namespace po = boost::program_options;

using namespace std;
using namespace sdpa::tests;

#define NO_GUI ""

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("0"); return s; }

typedef sdpa::nre::worker::BasicWorkerClient TestWorkerClient;

#ifdef USE_REAL_WE
	typedef sdpa::nre::worker::NreWorkerClient WorkerClient;
#else
	typedef sdpa::nre::worker::BasicWorkerClient WorkerClient;
#endif

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
			, m_pool (0)
	    	, m_kvsd (0)
	    	, m_serv (0)
	    	, m_thrd (0)
			, m_arrAggMasterInfo(1, MasterInfo("orchestrator_0"))
			, m_arrNreMasterInfo(1, MasterInfo("aggregator_0"))
	{ //initialize and start_agent the finite state machine

		FHGLOG_SETUP();

#ifdef USE_REAL_WE
		m_bLaunchNrePcd = true;
#else
		m_bLaunchNrePcd = false;
#endif

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

		sstrOrch.str("");
		sstrAgg.str("");
		sstrNRE.str("");

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

	void run_client();

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
	sdpa::master_info_list_t m_arrNreMasterInfo;

	std::stringstream sstrOrch;
	std::stringstream sstrAgg;
	std::stringstream sstrNRE;

	boost::thread m_threadClient;
	bool m_bLaunchNrePcd;
	pid_t pidPcd_;
};

void MyFixture::run_client()
{
	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config );
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

		LOG( DEBUG, "*****JOB #"<<k<<"******");

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

				boost::this_thread::sleep(boost::posix_time::seconds(10));
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

				boost::this_thread::sleep(boost::posix_time::seconds(3));
			}
		}

		nTrials = 0;

		try {
				LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
				ptrCli->retrieveResults(job_id_user);
				boost::this_thread::sleep(boost::posix_time::seconds(3));
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			if(nTrials++ > NMAXTRIALS)
			{
				LOG( DEBUG, "The maximum number of trials was exceeded. Giving-up now!");

				ptrCli->shutdown_network();
				ptrCli.reset();
				return;
			}

			boost::this_thread::sleep(boost::posix_time::seconds(3));
		}

		nTrials = 0;

		try {
			LOG( DEBUG, "User: delete the job "<<job_id_user);
			ptrCli->deleteJob(job_id_user);
			boost::this_thread::sleep(boost::posix_time::seconds(3));
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			if(nTrials++ > NMAXTRIALS)
			{
				LOG( DEBUG, "The maximum number of  trials was exceeded. Giving-up now!");

				ptrCli->shutdown_network();
				ptrCli.reset();
				return;
			}

			boost::this_thread::sleep(boost::posix_time::seconds(3));
		}
	}

	ptrCli->shutdown_network();
	boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
    ptrCli.reset();
}

BOOST_FIXTURE_TEST_SUITE( test_agents, MyFixture )

BOOST_AUTO_TEST_CASE( testOrchestratorWithNoWe_Push )
{
	LOG( DEBUG, "***** testOrchestratorNoWe *****"<<std::endl);
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgg 		= "127.0.0.1";
	string addrNRE 		= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	//LOG( DEBUG, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	//LOG( DEBUG, "Create the Aggregator ...");

	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create(  "aggregator_0",
																											addrAgg,
																											m_arrAggMasterInfo,
																											MAX_CAP );
	ptrAgg->start_agent(false);

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	// use external scheduler and real GWES
	//LOG( DEBUG, "Create the NRE ...");


	sdpa::daemon::NRE<WorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NREFactory< RealWorkflowEngine,
											 WorkerClient>::create("NRE_0",
				                             addrNRE,
				                             m_arrNreMasterInfo,
				                             2,
				                             workerUrl,
				                             //strAppGuiUrl,
				                             guiUrl,
				                             m_bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		ptrNRE_0->start_agent(false);
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start_agent NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a Restart_agentStrategy->restart_agent()" );

		ptrNRE_0->shutdown();
		ptrAgg->shutdown();
		ptrOrch->shutdown();

		return;
	}


	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );


	ptrNRE_0->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testOrchestratorNoWe terminated!");
}


BOOST_AUTO_TEST_CASE( testOrchestratorEmptyWe_Push )
{
	LOG( DEBUG, "***** testOrchestratorNoWe *****"<<std::endl);
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgg 		= "127.0.0.1";
	string addrNRE 		= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	//LOG( DEBUG, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<EmptyWorkflowEngine>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	//LOG( DEBUG, "Create the Aggregator ...");

	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create(  "aggregator_0",
																											addrAgg,
																											m_arrAggMasterInfo,
																											MAX_CAP );
	ptrAgg->start_agent(false);

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	// use external scheduler and real GWES
	//LOG( DEBUG, "Create the NRE ...");


	sdpa::daemon::NRE<WorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NREFactory< RealWorkflowEngine,
											 WorkerClient>::create("NRE_0",
				                             addrNRE,
				                             m_arrNreMasterInfo,
				                             2,
				                             workerUrl,
				                             //strAppGuiUrl,
				                             guiUrl,
				                             m_bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		ptrNRE_0->start_agent(false);
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start_agent NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a Restart_agentStrategy->restart_agent()" );

		ptrNRE_0->shutdown();
		ptrAgg->shutdown();
		ptrOrch->shutdown();

		return;
	}


	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );


	ptrNRE_0->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testOrchestratorNoWe terminated!");
}

BOOST_AUTO_TEST_CASE( testOrchestratorWithNoWe_Req )
{
	LOG( DEBUG, "***** testOrchestratorNoWe *****"<<std::endl);
	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgg 		= "127.0.0.1";
	string addrNRE 		= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	//LOG( DEBUG, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent();

	//LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create(	"aggregator_0",
																											addrAgg,
																											m_arrAggMasterInfo,
																											MAX_CAP);
	ptrAgg->start_agent();

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	// use external scheduler and real GWES
	//LOG( DEBUG, "Create the NRE ...");
	sdpa::daemon::NRE<WorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NREFactory<RealWorkflowEngine, WorkerClient>::create("NRE_0",
				                             addrNRE,
				                             m_arrNreMasterInfo,
				                             2,
				                             workerUrl,
				                             //strAppGuiUrl,
				                             guiUrl,
				                             m_bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		ptrNRE_0->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start_agent NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a Restart_agentStrategy->restart_agent()" );

		ptrNRE_0->shutdown();
		ptrAgg->shutdown();
		ptrOrch->shutdown();

		return;
	}

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	ptrNRE_0->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testOrchestratorNoWe terminated!");
}

BOOST_AUTO_TEST_CASE( testOrchestratorEmptyWe_Req )
{
	LOG( DEBUG, "***** testOrchestratorEmptyWe *****"<<std::endl);
	//guiUrl
	string guiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	//LOG( DEBUG, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<EmptyWorkflowEngine>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent();

	//LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg, m_arrAggMasterInfo, MAX_CAP);
	ptrAgg->start_agent();

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	// use external scheduler and real GWES
	//LOG( DEBUG, "Create the NRE ...");
	sdpa::daemon::NRE<WorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NREFactory<RealWorkflowEngine, WorkerClient>::create("NRE_0",
				                             addrNRE,
				                             m_arrNreMasterInfo,
				                             2, //capacity of the NRE is set to 2
				                             workerUrl,
				                             //strAppGuiUrl,
				                             guiUrl,
				                             m_bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		ptrNRE_0->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start_agent NRE: " << ex.what());

		ptrNRE_0->shutdown();
		ptrAgg->shutdown();
		ptrOrch->shutdown();

		return;
	}

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	ptrNRE_0->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testOrchestratorEmptyWe terminated!");
}

BOOST_AUTO_TEST_SUITE_END()
