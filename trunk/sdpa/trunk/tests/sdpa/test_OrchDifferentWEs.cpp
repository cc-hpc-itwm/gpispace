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
#include <sdpa/daemon/nre/NRE.hpp>
#include <seda/StageRegistry.hpp>

#include <sdpa/daemon/nre/nre-worker/nre-worker/ActivityExecutor.hpp>
#include <sdpa/daemon/nre/messages.hpp>

#include <boost/filesystem/path.hpp>
#include <sys/wait.h>

#include <we/loader/module.hpp>
#include <sdpa/daemon/EmptyWorkflowEngine.hpp>
#include <we/mgmt/basic_layer.hpp>
#include <we/mgmt/layer.hpp>

namespace po = boost::program_options;

using namespace std;
using namespace sdpa::tests;

#define NO_GUI ""

typedef we::mgmt::layer<id_type, we::activity_t> RealWorkflowEngine;

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("12100"); return s; }

namespace sdpa { namespace tests { namespace worker {
  class NreWorkerClient
  {
  public:
    explicit
    NreWorkerClient( const std::string & /*nre_worker_location*/
                   , const bool /*bLaunchNrePcd*/ = false
                   , const char* /*szNrePcdBinPath*/ = ""
                   , const char* /*szKDMModulesPath*/ = ""
                   , const char* /*szFvmPCModule*/ = ""
                   )
    : SDPA_INIT_LOGGER("TestNreWorkerClient") { }

    void set_ping_interval(unsigned long /*seconds*/){}
    void set_ping_timeout(unsigned long /*seconds*/){}
    void set_ping_trials(std::size_t /*max_tries*/){}

    void set_location(const std::string &str_loc){ nre_worker_location_ = str_loc; }

    unsigned int start() throw (std::exception){ LOG( INFO, "Start the test NreWorkerClient ..."); return 0;}
    void stop() throw (std::exception) { LOG( INFO, "Stop the test NreWorkerClient ...");}

    void cancel() throw (std::exception){ throw std::runtime_error("not implemented"); }

    sdpa::nre::worker::execution_result_t execute(const encoded_type& in_activity, unsigned long walltime = 0) throw (sdpa::nre::worker::WalltimeExceeded, std::exception)
	{
    	LOG( INFO, "Execute the activity "<<in_activity);
    	LOG( INFO, "Report activity finished ...");
    	return std::make_pair(sdpa::nre::worker::ACTIVITY_FINISHED, "empty result");
	}

  private:
    std::string nre_worker_location_;
    SDPA_DECLARE_LOGGER();
  };
}}}

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
	{ //initialize and start the finite state machine

		FHGLOG_SETUP();

		LOG(DEBUG, "Fixture's constructor called ...");

		m_ptrPool = new fhg::com::io_service_pool(1);
		m_ptrKvsd = new fhg::com::kvs::server::kvsd ("/tmp/notthere");
		m_ptrServ = new fhg::com::tcp_server ( *m_ptrPool
										  , *m_ptrKvsd
										  , kvs_host ()
										  , kvs_port ()
										  , true
										  );

		m_ptrThrd = new boost::thread (boost::bind ( &fhg::com::io_service_pool::run
												, m_ptrPool
												)
								   );

		m_ptrServ->start();
		sleep (1);

		fhg::com::kvs::get_or_create_global_kvs ( kvs_host()
												, kvs_port()
												, true
												, boost::posix_time::seconds(10)
												, 3
												);
	}

	~MyFixture()
	{
		LOG(DEBUG, "Fixture's destructor called ...");
		//stop the finite state machine

		m_ptrPool->stop ();
		m_ptrThrd->join ();
		delete m_ptrThrd;
		delete m_ptrServ;
		delete m_ptrKvsd;
		delete m_ptrPool;
	}

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

	void startDaemons(const std::string& workerUrl);

	int m_nITER;
	int m_sleep_interval ;
    std::string m_strWorkflow;

	fhg::com::io_service_pool *m_ptrPool;
	fhg::com::kvs::server::kvsd *m_ptrKvsd;
	fhg::com::tcp_server *m_ptrServ;
	boost::thread *m_ptrThrd;
};

BOOST_FIXTURE_TEST_SUITE( test_orchestrator_empty_we, MyFixture )

BOOST_AUTO_TEST_CASE( testOrchestratorNoWe )
{
	LOG( DEBUG, "***** testOrchestratorEmptyWe *****"<<std::endl);
	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	typedef void OrchWorkflowEngine;

	bool bLaunchNrePcd = true;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	LOG( DEBUG, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, "workflows");
	sdpa::daemon::Orchestrator::start(ptrOrch);

	LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	sdpa::daemon::Aggregator::start(ptrAgg);

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	// use external scheduler and real GWES
	LOG( DEBUG, "Create the NRE ...");
	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::create("NRE_0",
				                             addrNRE,"aggregator_0",
				                             workerUrl,
				                             strGuiUrl,
				                             bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::start(ptrNRE_0);
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
		sdpa::daemon::Aggregator::shutdown(ptrAgg);
		sdpa::daemon::Orchestrator::shutdown(ptrOrch);

		return;
	}

	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config );
	ptrCli->configure_network( config );

	for( int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = ptrCli->submitJob(m_strWorkflow);

		LOG( DEBUG, "*****JOB #"<<k<<"******");

		std::string job_status = ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = ptrCli->queryJob(job_id_user);
			LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(5*m_sleep_interval);
		}

		LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
		ptrCli->retrieveResults(job_id_user);

		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
	}

	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
	sdpa::daemon::Aggregator::shutdown(ptrAgg);
	sdpa::daemon::Orchestrator::shutdown(ptrOrch);

	ptrCli->shutdown_network();
    ptrCli.reset();
	seda::StageRegistry::instance().stopAll();
	seda::StageRegistry::instance().clear();

	sleep(1);
	LOG( DEBUG, "The test case testOrchestratorEmptyWe terminated!");
}

BOOST_AUTO_TEST_CASE( testOrchestratorEmptyWe )
{
	LOG( DEBUG, "***** testOrchestratorEmptyWe *****"<<std::endl);
	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	bool bLaunchNrePcd = true;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	LOG( DEBUG, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<EmptyWorkflowEngine>::create("orchestrator_0", addrOrch, "workflows");
	sdpa::daemon::Orchestrator::start(ptrOrch);

	LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	sdpa::daemon::Aggregator::start(ptrAgg);

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	// use external scheduler and real GWES
	LOG( DEBUG, "Create the NRE ...");
	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::create("NRE_0",
				                             addrNRE,"aggregator_0",
				                             workerUrl,
				                             strGuiUrl,
				                             bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::start(ptrNRE_0);
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
		sdpa::daemon::Aggregator::shutdown(ptrAgg);
		sdpa::daemon::Orchestrator::shutdown(ptrOrch);

		return;
	}

	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config );
	ptrCli->configure_network( config );

	for( int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = ptrCli->submitJob(m_strWorkflow);

		LOG( DEBUG, "*****JOB #"<<k<<"******");

		std::string job_status = ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = ptrCli->queryJob(job_id_user);
			LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(5*m_sleep_interval);
		}

		LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
		ptrCli->retrieveResults(job_id_user);

		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
	}

	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
	sdpa::daemon::Aggregator::shutdown(ptrAgg);
	sdpa::daemon::Orchestrator::shutdown(ptrOrch);

	ptrCli->shutdown_network();
    ptrCli.reset();
	seda::StageRegistry::instance().stopAll();
	seda::StageRegistry::instance().clear();

	sleep(1);
	LOG( DEBUG, "The test case testOrchestratorEmptyWe terminated!");
}

BOOST_AUTO_TEST_CASE( testOrchestratorRealWe )
{
	LOG( DEBUG, "***** testOrchestratorRealWe *****"<<std::endl);
	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	bool bLaunchNrePcd = true;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	LOG( DEBUG, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<RealWorkflowEngine>::create("orchestrator_0", addrOrch, "workflows");
	sdpa::daemon::Orchestrator::start(ptrOrch);

	LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	sdpa::daemon::Aggregator::start(ptrAgg);

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	// use external scheduler and real GWES
	LOG( DEBUG, "Create the NRE ...");
	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::create("NRE_0",
				                             addrNRE,"aggregator_0",
				                             workerUrl,
				                             strGuiUrl,
				                             bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::start(ptrNRE_0);
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
		sdpa::daemon::Aggregator::shutdown(ptrAgg);
		sdpa::daemon::Orchestrator::shutdown(ptrOrch);

		return;
	}

	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config );
	ptrCli->configure_network( config );

	for( int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = ptrCli->submitJob(m_strWorkflow);

		LOG( DEBUG, "*****JOB #"<<k<<"******");

		std::string job_status = ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = ptrCli->queryJob(job_id_user);
			LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(5*m_sleep_interval);
		}

		LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
		ptrCli->retrieveResults(job_id_user);

		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
	}

	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
	sdpa::daemon::Aggregator::shutdown(ptrAgg);
	sdpa::daemon::Orchestrator::shutdown(ptrOrch);

	ptrCli->shutdown_network();
    ptrCli.reset();
	seda::StageRegistry::instance().stopAll();
	seda::StageRegistry::instance().clear();

	sleep(1);
	LOG( DEBUG, "The test case testOrchestratorEmptyWe terminated!");
}

#if 0
BOOST_AUTO_TEST_CASE( testOrchestratorEmptyWe2Aggs )
{
	LOG( DEBUG, "***** testOrchestratorEmptyWe *****"<<std::endl);
	string strGuiUrl = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	bool bLaunchNrePcd = true;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	LOG( DEBUG, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<EmptyWorkflowEngine>::create("orchestrator_0", addrOrch, "workflows");
	sdpa::daemon::Orchestrator::start(ptrOrch);

	LOG( DEBUG, "Create the Aggregator 0 ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	sdpa::daemon::Aggregator::start(ptrAgg);

	LOG( DEBUG, "Create the Aggregator 1 ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg_1 = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_1", addrAgg,"aggregator_0");
	sdpa::daemon::Aggregator::start(ptrAgg_1);

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	// use external scheduler and real GWES
	LOG( DEBUG, "Create the NRE ...");
	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::create("NRE_0",
				                             addrNRE,"aggregator_1",
				                             workerUrl,
				                             strGuiUrl,
				                             bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::start(ptrNRE_0);
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
		sdpa::daemon::Aggregator::shutdown(ptrAgg);
		sdpa::daemon::Aggregator::shutdown(ptrAgg_1);
		sdpa::daemon::Orchestrator::shutdown(ptrOrch);

		return;
	}

	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config );
	ptrCli->configure_network( config );

	for( int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = ptrCli->submitJob(m_strWorkflow);

		LOG( DEBUG, "*****JOB #"<<k<<"******");

		std::string job_status = ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = ptrCli->queryJob(job_id_user);
			LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(5*m_sleep_interval);
		}

		LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
		ptrCli->retrieveResults(job_id_user);

		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
	}

	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
	sdpa::daemon::Aggregator::shutdown(ptrAgg);
	sdpa::daemon::Aggregator::shutdown(ptrAgg_1);
	sdpa::daemon::Orchestrator::shutdown(ptrOrch);

	ptrCli->shutdown_network();
    ptrCli.reset();
	seda::StageRegistry::instance().stopAll();
	seda::StageRegistry::instance().clear();

	sleep(1);
	LOG( DEBUG, "The test case testOrchestratorEmptyWe terminated!");
}
#endif

BOOST_AUTO_TEST_SUITE_END()
