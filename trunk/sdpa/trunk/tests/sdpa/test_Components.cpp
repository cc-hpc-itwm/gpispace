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
#include "test_Components.hpp"
#include "tests_config.hpp"

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

CPPUNIT_TEST_SUITE_REGISTRATION( TestComponents );

TestComponents::TestComponents() :
	SDPA_INIT_LOGGER("sdpa.tests.TestComponents"),
    m_nITER(1),
    m_sleep_interval(1000000)
{
}

TestComponents::~TestComponents()
{
  	kill(0,SIGTERM);
	SDPA_LOG_INFO("The test suite finished!");
}

string TestComponents::read_workflow(string strFileName)
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

    unsigned int start() throw (std::exception){ SDPA_LOG_INFO("Start the test NreWorkerClient ..."); return 0;}
    void stop() throw (std::exception) { SDPA_LOG_INFO("Stop the test NreWorkerClient ...");}

    void cancel() throw (std::exception){ throw std::runtime_error("not implemented"); }

    sdpa::nre::worker::execution_result_t execute(const encoded_type& in_activity, unsigned long walltime = 0) throw (sdpa::nre::worker::WalltimeExceeded, std::exception)
	{
    	SDPA_LOG_INFO("Execute the activity "<<in_activity);
    	SDPA_LOG_INFO("Report activity finished ...");
    	return std::make_pair(sdpa::nre::worker::ACTIVITY_FINISHED, "empty result");
	}

  private:
    std::string nre_worker_location_;
    SDPA_DECLARE_LOGGER();
  };
}}}


void TestComponents::setUp()
{ //initialize and start the finite state machine
	SDPA_LOG_DEBUG("setUP");

	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
    cav.push_back("--orchestrator=orchestrator_0");
    cav.push_back("--network.location=orchestrator_0:127.0.0.1:5000");
    config.parse_command_line(cav);

	m_ptrCli = sdpa::client::ClientApi::create( config );
	m_ptrCli->configure_network( config );

	seda::Stage::Ptr user_stage = seda::StageRegistry::instance().lookup(m_ptrCli->input_stage());
}

void TestComponents::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
	//stop the finite state machine

	m_ptrCli->shutdown_network();
	m_ptrCli.reset();
	seda::StageRegistry::instance().clear();
}


void TestComponents::testActivityRealWeAllCompAndNreWorkerSpawnedByNRE()
{
	SDPA_LOG_DEBUG("***** testActivityRealWeAllCompAndNreWorkerSpawnedByNRE *****"<<std::endl);
	string strGuiUrl = "";
	string workerUrl = "127.0.0.1:12500";
	string orchestratorPort = "127.0.0.1:12000";
	string aggregatorPort = "127.0.0.1:12001";
	string nrePort = "127.0.0.1:12002";

	bool bLaunchNrePcd = true;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);

	SDPA_LOG_DEBUG("Create the Orchestrator ...");
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::ptr_t ptrOrch = sdpa::daemon::Orchestrator<RealWorkflowEngine>::create("orchestrator_0", orchestratorPort, "workflows");
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::start(ptrOrch);

	SDPA_LOG_DEBUG("Create the Aggregator ...");
	sdpa::daemon::Aggregator<RealWorkflowEngine>::ptr_t ptrAgg = sdpa::daemon::Aggregator<RealWorkflowEngine>::create("aggregator_0", aggregatorPort,"orchestrator_0", orchestratorPort);
	sdpa::daemon::Aggregator<RealWorkflowEngine>::start(ptrAgg);

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	// use external scheduler and real GWES
	SDPA_LOG_DEBUG("Create the NRE ...");
	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::create("NRE_0",
				                             nrePort,"aggregator_0", aggregatorPort, workerUrl, strGuiUrl,
				                             bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::start(ptrNRE_0);
	}
	catch (const std::exception &ex) {
		SDPA_LOG_FATAL("Could not start NRE: " << ex.what());
		SDPA_LOG_WARN("TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
		sdpa::daemon::Aggregator<RealWorkflowEngine>::shutdown(ptrAgg);
		sdpa::daemon::Orchestrator<RealWorkflowEngine>::shutdown(ptrOrch);

		return;
	}

	for( int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrCli->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("*****JOB #"<<k<<"******");

		std::string job_status =  m_ptrCli->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrCli->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrCli->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrCli->deleteJob(job_id_user);
	}

	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
	sdpa::daemon::Aggregator<RealWorkflowEngine>::shutdown(ptrAgg);
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::shutdown(ptrOrch);

	sleep(1);
	SDPA_LOG_DEBUG("testActivityRealWeAllCompAndNreWorkerSpywnedByNRE finished!");
}


void TestComponents::testActivityRealWeAllCompAndNreWorkerSpawnedByTest()
{
	SDPA_LOG_DEBUG("***** testActivityRealWeAllCompAndNreWorkerSpawnedByTest *****"<<std::endl);

	string workerUrl = "127.0.0.1:12500";

	startPcdAndDaemons(workerUrl);
	SDPA_LOG_DEBUG("testActivityRealWeAllCompAndNreWorkerSpawnedByTest finished!");
}

void TestComponents::startPcdAndDaemons(const std::string& workerUrl) throw (std::exception)
{
   	int c;
   	int nStatus;
   	string strID;

   	pid_t pID = fork();
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
	  	kill( pID, SIGKILL);

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
}


void TestComponents::startDaemons(const std::string& workerUrl)
{
	string strGuiUrl = "";
	string orchestratorPort = "127.0.0.1:12000";
	string aggregatorPort = "127.0.0.1:12001";
	string nrePort = "127.0.0.1:12002";
	bool bLaunchNrePcd = false;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);

	SDPA_LOG_DEBUG("Create the Orchestrator ...");
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::ptr_t ptrOrch = sdpa::daemon::Orchestrator<RealWorkflowEngine>::create("orchestrator_0", orchestratorPort, "workflows");
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::start(ptrOrch);

	SDPA_LOG_DEBUG("Create the Aggregator ...");
	sdpa::daemon::Aggregator<RealWorkflowEngine>::ptr_t ptrAgg = sdpa::daemon::Aggregator<RealWorkflowEngine>::create("aggregator_0", aggregatorPort,"orchestrator_0", orchestratorPort);
	sdpa::daemon::Aggregator<RealWorkflowEngine>::start(ptrAgg);

	// use external scheduler and real GWES
	SDPA_LOG_DEBUG("Create the NRE ...");
	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::create("NRE_0",
											 nrePort,"aggregator_0", aggregatorPort, workerUrl, strGuiUrl );


	try {
		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::start(ptrNRE_0);
	}
	catch (const std::exception &ex) {
		SDPA_LOG_FATAL("Could not start NRE: " << ex.what());
		SDPA_LOG_WARN("TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		sdpa::daemon::Orchestrator<RealWorkflowEngine>::shutdown(ptrOrch);
		sdpa::daemon::Aggregator<RealWorkflowEngine>::shutdown(ptrAgg);
		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);

		return;
	}

	for( int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrCli->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("*****JOB #"<<k<<"******");

		std::string job_status =  m_ptrCli->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrCli->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrCli->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrCli->deleteJob(job_id_user);
	}

	sdpa::daemon::Orchestrator<RealWorkflowEngine>::shutdown(ptrOrch);
	sdpa::daemon::Aggregator<RealWorkflowEngine>::shutdown(ptrAgg);
	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);

	sleep(1);
}


void TestComponents::testActivityRealWeAllCompActExec()
{
	SDPA_LOG_DEBUG("***** testActivityRealWeAllCompAndActExec *****"<<std::endl);
	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:12500";
	string orchestratorPort = "127.0.0.1:12000";
	string aggregatorPort = "127.0.0.1:12001";
	string nrePort = "127.0.0.1:12002";
	bool bLaunchNrePcd = false;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);

	SDPA_LOG_DEBUG("Create the Orchestrator ...");
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::ptr_t ptrOrch = sdpa::daemon::Orchestrator<RealWorkflowEngine>::create("orchestrator_0", orchestratorPort, "workflows");
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::start(ptrOrch);

	SDPA_LOG_DEBUG("Create the Aggregator ...");
	sdpa::daemon::Aggregator<RealWorkflowEngine>::ptr_t ptrAgg = sdpa::daemon::Aggregator<RealWorkflowEngine>::create("aggregator_0", aggregatorPort,"orchestrator_0", orchestratorPort);
	sdpa::daemon::Aggregator<RealWorkflowEngine>::start(ptrAgg);

	// use external scheduler and dummy WE
	SDPA_LOG_DEBUG("Create the NRE ...");
	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::create("NRE_0", nrePort,"aggregator_0", aggregatorPort, workerUrl, strGuiUrl );

	SDPA_LOG_DEBUG("starting process container on location: "<<workerUrl<< std::endl);
	sdpa::shared_ptr<sdpa::nre::worker::ActivityExecutor> executor(new sdpa::nre::worker::ActivityExecutor(workerUrl));
	executor->loader().append_search_path (TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	try {
		SDPA_LOG_INFO("Load the fake-fvm module ("<<TESTS_FVM_PC_FAKE_MODULE<<") ...");
		boost::filesystem::path pathFakeFvmModule(TESTS_FVM_PC_FAKE_MODULE);
		executor->loader().load("fvm", pathFakeFvmModule);
	}
	catch(const we::loader::ModuleLoadFailed& ex)
	{
		 SDPA_LOG_ERROR ("Could not load the module "<<TESTS_FVM_PC_FAKE_MODULE<<"!!!");
		 CPPUNIT_ASSERT (false);
	}

	try {
		executor->start();
	}
	catch (const std::exception &ex) {
	  SDPA_LOG_ERROR ("could not start executor: " << ex.what());
	  CPPUNIT_ASSERT (false);
	}

	SDPA_LOG_DEBUG("starting the NRE ...");
	try {
		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::start(ptrNRE_0);
	}
	catch (const std::exception &ex) {
		SDPA_LOG_FATAL("Could not start NRE: " << ex.what());
		SDPA_LOG_WARN("TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		sdpa::daemon::Orchestrator<RealWorkflowEngine>::shutdown(ptrOrch);
		sdpa::daemon::Aggregator<RealWorkflowEngine>::shutdown(ptrAgg);
		sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);

		return;
	}

	for( int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrCli->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("*****JOB #"<<k<<"******");

		std::string job_status =  m_ptrCli->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrCli->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrCli->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrCli->deleteJob(job_id_user);
	}

	sdpa::daemon::NRE<RealWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
	sdpa::daemon::Aggregator<RealWorkflowEngine>::shutdown(ptrAgg);
	sdpa::daemon::Orchestrator<RealWorkflowEngine>::shutdown(ptrOrch);

	// processor container terminates ...
	//fvm_pc.leave();
	SDPA_LOG_INFO("terminating...");
	if (! executor->stop())
		SDPA_LOG_WARN("executor did not stop correctly...");

	sleep(1);
	SDPA_LOG_DEBUG("testActivityRealWeAllCompAndActExec with fvm-pc finished!");
}


void TestComponents::testActivityDummyWeAllCompActExec()
{
	SDPA_LOG_DEBUG("***** testActivityDummyWeAllCompAndNreWorker *****"<<std::endl);
	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:12500";
	string orchestratorPort = "127.0.0.1:12000";
	string aggregatorPort = "127.0.0.1:12001";
	string nrePort = "127.0.0.1:12002";
	bool bLaunchNrePcd = false;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	SDPA_LOG_DEBUG("The test workflow is "<<m_strWorkflow);

	SDPA_LOG_DEBUG("Create the Orchestrator ...");
	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::ptr_t ptrOrch = sdpa::daemon::Orchestrator<DummyWorkflowEngine>::create("orchestrator_0", orchestratorPort, "workflows");
	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::start(ptrOrch);

	SDPA_LOG_DEBUG("Create the Aggregator ...");
	sdpa::daemon::Aggregator<DummyWorkflowEngine>::ptr_t ptrAgg = sdpa::daemon::Aggregator<DummyWorkflowEngine>::create("aggregator_0", aggregatorPort,"orchestrator_0", orchestratorPort);
	sdpa::daemon::Aggregator<DummyWorkflowEngine>::start(ptrAgg);

	// use external scheduler and dummy WE
	SDPA_LOG_DEBUG("Create the NRE ...");
	sdpa::daemon::NRE<DummyWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NRE<DummyWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::create("NRE_0", nrePort,"aggregator_0", aggregatorPort, workerUrl, strGuiUrl );

	SDPA_LOG_DEBUG("starting process container on location: "<<workerUrl<< std::endl);
	sdpa::shared_ptr<sdpa::nre::worker::ActivityExecutor> executor(new sdpa::nre::worker::ActivityExecutor(workerUrl));
    executor->loader().append_search_path (TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

    try {
		SDPA_LOG_INFO("Load the fake-fvm module ("<<TESTS_FVM_PC_FAKE_MODULE<<") ...");
		boost::filesystem::path pathFakeFvmModule(TESTS_FVM_PC_FAKE_MODULE);
		executor->loader().load("fvm", pathFakeFvmModule);
	}
	catch(const we::loader::ModuleLoadFailed& ex)
	{
		 SDPA_LOG_ERROR ("Could not load the module "<<TESTS_FVM_PC_FAKE_MODULE<<"!!!");
		 CPPUNIT_ASSERT (false);
	}

	try {
		executor->start();
	}
	catch (const std::exception &ex) {
	  SDPA_LOG_ERROR ("could not start executor: " << ex.what());
	  CPPUNIT_ASSERT (false);
	}

	try {
		sdpa::daemon::NRE<DummyWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::start(ptrNRE_0);
	}
	catch (const std::exception &ex) {
		SDPA_LOG_FATAL("Could not start NRE: " << ex.what());
		SDPA_LOG_WARN("TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		sdpa::daemon::Orchestrator<DummyWorkflowEngine>::shutdown(ptrOrch);
		sdpa::daemon::Aggregator<DummyWorkflowEngine>::shutdown(ptrAgg);
		sdpa::daemon::NRE<DummyWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);

		return;
	}

	for( int k=0; k<m_nITER; k++ )
	{
		sdpa::job_id_t job_id_user = m_ptrCli->submitJob(m_strWorkflow);

		SDPA_LOG_DEBUG("*****JOB #"<<k<<"******");

		std::string job_status =  m_ptrCli->queryJob(job_id_user);
		SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = m_ptrCli->queryJob(job_id_user);
			SDPA_LOG_DEBUG("The status of the job "<<job_id_user<<" is "<<job_status);

			usleep(m_sleep_interval);
		}

		SDPA_LOG_DEBUG("User: retrieve results of the job "<<job_id_user);
		m_ptrCli->retrieveResults(job_id_user);

		SDPA_LOG_DEBUG("User: delete the job "<<job_id_user);
		m_ptrCli->deleteJob(job_id_user);
	}

	sdpa::daemon::NRE<DummyWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::shutdown(ptrNRE_0);
	sdpa::daemon::Aggregator<DummyWorkflowEngine>::shutdown(ptrAgg);
	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::shutdown(ptrOrch);

	SDPA_LOG_INFO("terminating...");
	if (! executor->stop())
		SDPA_LOG_WARN("executor did not stop correctly...");

	sleep(1);
	SDPA_LOG_DEBUG("testActivityDummyWeAllCompAndNreWorker finished!");
}
