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
#define BOOST_TEST_MODULE TestAgentInteraction
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
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/aggregator/Aggregator.hpp>
#include <sdpa/daemon/nre/NRE.hpp>
#include <seda/StageRegistry.hpp>
#include <tests/sdpa/DummyWorkflowEngine.hpp>

#include <sdpa/daemon/nre/nre-worker/nre-worker/ActivityExecutor.hpp>
#include <sdpa/daemon/nre/messages.hpp>

#include <boost/filesystem/path.hpp>
#include <sys/wait.h>

#include <we/loader/module.hpp>

namespace po = boost::program_options;

using namespace std;
using namespace sdpa::tests;

#define NO_GUI ""

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


void MyFixture::startDaemons(const std::string& workerUrl)
{
	string strGuiUrl = "";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";
	//bool bLaunchNrePcd = false;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	LOG( DEBUG, "Create the Orchestrator ...");
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::ptr_t ptrOrch = sdpa::daemon::Orchestrator<RealWorkflowEngine>::create("orchestrator_0", addrOrch, "workflows");
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::start(ptrOrch);

	LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator<RealWorkflowEngine>::ptr_t ptrAgg = sdpa::daemon::Aggregator<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	sdpa::daemon::Aggregator<RealWorkflowEngine>::start(ptrAgg);

	// use external scheduler and real GWES
	LOG( DEBUG, "Create the NRE ...");
	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::create("NRE_0",
											 addrNRE,"aggregator_0", workerUrl, strGuiUrl );


	try {
		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::start(ptrNRE_0);
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		sdpa::daemon::Orchestrator<RealWorkflowEngine>::shutdown(ptrOrch);
		sdpa::daemon::Aggregator<RealWorkflowEngine>::shutdown(ptrAgg);
		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);

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

		std::string job_status =  ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = ptrCli->queryJob(job_id_user);
			LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
		ptrCli->retrieveResults(job_id_user);

		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
	}

	sdpa::daemon::Orchestrator<RealWorkflowEngine>::shutdown(ptrOrch);
	sdpa::daemon::Aggregator<RealWorkflowEngine>::shutdown(ptrAgg);
	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);

	ptrCli->shutdown_network();
	ptrCli.reset();
	seda::StageRegistry::instance().stopAll();
	seda::StageRegistry::instance().clear();

	sleep(1);
}

BOOST_FIXTURE_TEST_SUITE( test_suite_agent_int, MyFixture )

BOOST_AUTO_TEST_CASE( testActivityRealWeAllCompAndNreWorkerSpawnedByNRE )
{
	LOG( DEBUG, "***** testActivityRealWeAllCompAndNreWorkerSpawnedByNRE *****"<<std::endl);
	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	bool bLaunchNrePcd = true;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	LOG( DEBUG, "Create the Orchestrator ...");
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::ptr_t ptrOrch = sdpa::daemon::Orchestrator<RealWorkflowEngine>::create("orchestrator_0", addrOrch, "workflows");
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::start(ptrOrch);

	LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator<RealWorkflowEngine>::ptr_t ptrAgg = sdpa::daemon::Aggregator<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	sdpa::daemon::Aggregator<RealWorkflowEngine>::start(ptrAgg);

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
		sdpa::daemon::Aggregator<RealWorkflowEngine>::shutdown(ptrAgg);
		sdpa::daemon::Orchestrator<RealWorkflowEngine>::shutdown(ptrOrch);

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
	sdpa::daemon::Aggregator<RealWorkflowEngine>::shutdown(ptrAgg);
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::shutdown(ptrOrch);


	ptrCli->shutdown_network();
    ptrCli.reset();
	seda::StageRegistry::instance().stopAll();
	seda::StageRegistry::instance().clear();

	sleep(1);
	LOG( DEBUG, "The test case testActivityRealWeAllCompAndNreWorkerSpywnedByNRE terminated!");
}

#if 0
BOOST_AUTO_TEST_CASE( testActivityRealWeAllCompAndNreWorkerSpawnedByTest )
{
	LOG( DEBUG, "***** testActivityRealWeAllCompAndNreWorkerSpawnedByTest *****"<<std::endl);

	string workerUrl = "127.0.0.1:12500";

	int c;
   	int nStatus;
   	string strID;

   	pid_t pID = vfork();
   	LOG(INFO, "Try to launch the nre-pcd ...");

   	if (pID == 0)  // child
   	{
		// Code only executed by child process

		strID = "nre-pcd: ";

	    execl(  TESTS_NRE_PCD_BIN_PATH,
	            "nre-pcd",
				"-l", workerUrl.c_str(),
				"-a", TESTS_EXAMPLE_STRESSTEST_MODULES_PATH,
				"--load", TESTS_FVM_PC_FAKE_MODULE,
				NULL );
	}
	else if (pID < 0)            // failed to fork
	{
		LOG(ERROR, "Failed to fork!");
		exit(1);
		// Throw exception
	}
	else                                   // parent
	{
	  	// Code only executed by parent process
	   	strID = "NREWorkerClient";
	  	startDaemons(workerUrl);

	  	//kill pcd here
	  	kill( pID, SIGTERM );

		c = wait(&nStatus);
	   	if( WIFEXITED(nStatus) )
	   	{
	   		if( WEXITSTATUS(nStatus) != 0 )
	   		{
	   			std::cerr<<"nre-pcd exited with the return code "<<(int)WEXITSTATUS(nStatus)<<endl;
	   			LOG(ERROR, "nre-pcd exited with the return code "<<(int)WEXITSTATUS(nStatus));
	   		}
	   		else
	   			if( WIFSIGNALED(nStatus) )
	   			{
	   				std::cerr<<"nre-pcd exited due to a signal: " <<(int)WTERMSIG(nStatus)<<endl;
	   				LOG(ERROR, "nre-pcd exited due to a signal: "<<(int)WTERMSIG(nStatus));
	   			}
	   	}
	}

	LOG( DEBUG, "The test case testActivityRealWeAllCompAndNreWorkerSpawnedByTest terminated!");
}

BOOST_AUTO_TEST_CASE( testActivityRealWeAllCompActExec )
{
	LOG( DEBUG, "***** testActivityRealWeAllCompAndActExec *****"<<std::endl);
	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:12500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";
	//bool bLaunchNrePcd = false;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	LOG( DEBUG, "Create the Orchestrator ...");
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::ptr_t ptrOrch = sdpa::daemon::Orchestrator<RealWorkflowEngine>::create("orchestrator_0", addrOrch, "workflows");
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::start(ptrOrch);

	LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator<RealWorkflowEngine>::ptr_t ptrAgg = sdpa::daemon::Aggregator<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	sdpa::daemon::Aggregator<RealWorkflowEngine>::start(ptrAgg);

	// use external scheduler and dummy WE
	LOG( DEBUG, "Create the NRE ...");
	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::create("NRE_0", addrNRE,"aggregator_0", workerUrl, strGuiUrl );

	LOG( DEBUG, "starting process container on location: "<<workerUrl<< std::endl);
	sdpa::shared_ptr<sdpa::nre::worker::ActivityExecutor> executor(new sdpa::nre::worker::ActivityExecutor(workerUrl));
	executor->loader().append_search_path (TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	try {
		LOG( INFO, "Load the fake-fvm module ("<<TESTS_FVM_PC_FAKE_MODULE<<") ...");
		boost::filesystem::path pathFakeFvmModule(TESTS_FVM_PC_FAKE_MODULE);
		executor->loader().load("fvm", pathFakeFvmModule);
	}
	catch(const we::loader::ModuleLoadFailed& ex)
	{
		 LOG( ERROR, "Could not load the module "<<TESTS_FVM_PC_FAKE_MODULE<<"!!!");
		 BOOST_ASSERT (false);
	}

	try {
		executor->start();
	}
	catch (const std::exception &ex) {
	  LOG( ERROR, "could not start executor: " << ex.what());
	  BOOST_ASSERT (false);
	}

	LOG( DEBUG, "starting the NRE ...");
	try {
		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::start(ptrNRE_0);
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		sdpa::daemon::Orchestrator<RealWorkflowEngine>::shutdown(ptrOrch);
		sdpa::daemon::Aggregator<RealWorkflowEngine>::shutdown(ptrAgg);
		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);

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

		std::string job_status =  ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = ptrCli->queryJob(job_id_user);
			LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
		ptrCli->retrieveResults(job_id_user);

		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
	}

	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
	sdpa::daemon::Aggregator<RealWorkflowEngine>::shutdown(ptrAgg);
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::shutdown(ptrOrch);

	// process container terminates ...
	LOG( INFO, "terminating...");
	if (! executor->stop())
		LOG( WARN, "executor did not stop correctly...");

	ptrCli->shutdown_network();
	ptrCli.reset();
	seda::StageRegistry::instance().stopAll();
	seda::StageRegistry::instance().clear();

	sleep(1);
	LOG( DEBUG, "The test case testActivityRealWeAllCompAndActExec with fvm-pc terminated!");
}

BOOST_AUTO_TEST_CASE( testActivityDummyWeAllCompActExec )
{
	LOG( DEBUG, "***** testActivityDummyWeAllCompAndNreWorker *****"<<std::endl);
	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:12500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";
	//bool bLaunchNrePcd = false;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	LOG( DEBUG, "Create the Orchestrator ...");
	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::ptr_t ptrOrch = sdpa::daemon::Orchestrator<DummyWorkflowEngine>::create("orchestrator_0", addrOrch, "workflows");
	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::start(ptrOrch);

	LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator<DummyWorkflowEngine>::ptr_t ptrAgg = sdpa::daemon::Aggregator<DummyWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	sdpa::daemon::Aggregator<DummyWorkflowEngine>::start(ptrAgg);

	// use external scheduler and dummy WE
	LOG( DEBUG, "Create the NRE ...");
	sdpa::daemon::NRE<DummyWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NRE<DummyWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::create("NRE_0", addrNRE,"aggregator_0", workerUrl, strGuiUrl );

	LOG( DEBUG, "starting process container on location: "<<workerUrl<< std::endl);
	sdpa::shared_ptr<sdpa::nre::worker::ActivityExecutor> executor(new sdpa::nre::worker::ActivityExecutor(workerUrl));
    executor->loader().append_search_path (TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

    try {
		LOG( INFO, "Load the fake-fvm module ("<<TESTS_FVM_PC_FAKE_MODULE<<") ...");
		boost::filesystem::path pathFakeFvmModule(TESTS_FVM_PC_FAKE_MODULE);
		executor->loader().load("fvm", pathFakeFvmModule);
	}
	catch(const we::loader::ModuleLoadFailed& ex)
	{
		 LOG( ERROR, "Could not load the module "<<TESTS_FVM_PC_FAKE_MODULE<<"!!!");
		 BOOST_ASSERT (false);
	}

	try {
		executor->start();
	}
	catch (const std::exception &ex) {
	  LOG( ERROR, "could not start executor: " << ex.what());
	  BOOST_ASSERT (false);
	}

	try {
		sdpa::daemon::NRE<DummyWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::start(ptrNRE_0);
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		sdpa::daemon::Orchestrator<DummyWorkflowEngine>::shutdown(ptrOrch);
		sdpa::daemon::Aggregator<DummyWorkflowEngine>::shutdown(ptrAgg);
		sdpa::daemon::NRE<DummyWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);

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

		std::string job_status =  ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = ptrCli->queryJob(job_id_user);
			LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
		ptrCli->retrieveResults(job_id_user);

		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
	}

	sdpa::daemon::NRE<DummyWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
	sdpa::daemon::Aggregator<DummyWorkflowEngine>::shutdown(ptrAgg);
	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::shutdown(ptrOrch);

	LOG( INFO, "terminating...");
	if (! executor->stop())
		LOG( WARN, "executor did not stop correctly...");

	ptrCli->shutdown_network();
	ptrCli.reset();
	seda::StageRegistry::instance().stopAll();
	seda::StageRegistry::instance().clear();

	sleep(1);
	LOG( DEBUG, "The test case testActivityDummyWeAllCompAndNreWorker terminated!");
}
#endif

BOOST_AUTO_TEST_SUITE_END()
