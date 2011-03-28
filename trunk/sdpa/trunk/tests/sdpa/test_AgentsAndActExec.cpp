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
#define BOOST_TEST_MODULE TestAgentsAndActExec
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

#include <sdpa/daemon/nre/nre-worker/nre-worker/ActivityExecutor.hpp>
#include <sdpa/daemon/nre/nre-worker/messages.hpp>

#include <boost/filesystem/path.hpp>
#include <sys/wait.h>

#include <sdpa/engine/DummyWorkflowEngine.hpp>
#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include <sdpa/engine/RealWorkflowEngine.hpp>

#ifdef USE_REAL_WE
	#include <sdpa/daemon/nre/nre-worker/NreWorkerClient.hpp>
#else
	#include <sdpa/daemon/nre/BasicWorkerClient.hpp>
#endif

const int NMAXTRIALS=3;

namespace po = boost::program_options;

using namespace std;
using namespace sdpa::tests;

#define NO_GUI ""

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("0"); return s; }

typedef sdpa::nre::worker::BasicWorkerClient TestWorkerClient;

#ifdef USE_REAL_WE
	typedef sdpa::nre::worker::NreWorkerClient WorkerClient;
	bool bLaunchNrePcd = true;
#else
	typedef sdpa::nre::worker::BasicWorkerClient WorkerClient;
	bool bLaunchNrePcd = false;
#endif

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
	{ //initialize and start the finite state machine

		FHGLOG_SETUP();

		LOG(DEBUG, "Fixture's constructor called ...");

		m_ptrPool = new fhg::com::io_service_pool(1);
		m_ptrKvsd = new fhg::com::kvs::server::kvsd ("");
		m_ptrServ = new fhg::com::tcp_server ( *m_ptrPool
										  , *m_ptrKvsd
										  , kvs_host ()
										  , kvs_port ()
										  , true
										  );
		m_ptrThrd = new boost::thread( boost::bind( &fhg::com::io_service_pool::run, m_ptrPool ) );

		m_ptrServ->start();

		LOG(INFO, "kvs daemon is listening on port " << m_ptrServ->port ());

		fhg::com::kvs::global::get_kvs_info().init( kvs_host()
												  , boost::lexical_cast<std::string>(m_ptrServ->port())
												  , boost::posix_time::seconds(10)
												  , 3
												  );
	}

	~MyFixture()
	{
		LOG(DEBUG, "Fixture's destructor called ...");

		m_ptrServ->stop ();
		m_ptrPool->stop ();
		m_ptrThrd->join ();

		delete m_ptrThrd;
		delete m_ptrServ;
		delete m_ptrKvsd;
		delete m_ptrPool;

		seda::StageRegistry::instance().stopAll();
		seda::StageRegistry::instance().clear();
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

	unsigned int startNrePcd( string &nre_worker_location_, string nre_pcd_binary_, vector<string> nre_pcd_search_path_, vector<string> nre_pcd_pre_load_ )
	{
		unsigned int rc = 0;

		// check here whether the nre-pcd is running or not
		LOG(INFO, "Try to spawn nre-pcd with fork!");
		pidPcd_ = fork();

		if (pidPcd_ == 0)  // child
		{
			// Code only executed by child process
			LOG(INFO, "After fork, I'm the child process ...");
			try {
				std::vector<std::string> cmdline;
				cmdline.push_back ( nre_pcd_binary_ );

				//LOG(INFO, std::string("nre_pcd_binary_ = ") + cmdline[0] );

				cmdline.push_back("-l");
				cmdline.push_back(nre_worker_location_.c_str());

				for( vector<string>::const_iterator it (nre_pcd_search_path_.begin())
				  ; it != nre_pcd_search_path_.end()
				  ; ++it
				  )
				{
					cmdline.push_back("--append-search-path");
					cmdline.push_back(*it);
				}

				for( vector<string>::const_iterator it (nre_pcd_pre_load_.begin()); it != nre_pcd_pre_load_.end(); ++it )
				{
					cmdline.push_back("--load");
					cmdline.push_back(*it);
				}

				char ** av = new char*[cmdline.size()+1];
				av[cmdline.size()] = (char*)(NULL);

				std::size_t idx (0);
				for ( std::vector<std::string>::const_iterator it (cmdline.begin())
				  ; it != cmdline.end()
				  ; ++it, ++idx
				  )
				{
					//LOG(INFO, std::string("cmdline[")<<idx<<"]=" << cmdline[idx] );

					av[idx] = new char[it->size()+1];
					memcpy(av[idx], it->c_str(), it->size());
					av[idx][it->size()] = (char)0;
				}

				std::stringstream sstr_cmd;
				for(size_t k=0; k<idx; k++)
					sstr_cmd << av[idx];

				LOG(DEBUG, std::string("Try to launch the nre-pcd with the following the command line:\n") + sstr_cmd.str() );

				if ( execv( nre_pcd_binary_.c_str(), av) < 0 )
				{
					throw std::runtime_error( std::string("could not exec command line ") + sstr_cmd.str() );
				}
				// not reached
			}
			catch(const std::exception& ex)
			{
				LOG(ERROR, "Exception occurred when trying to spawn nre-pcd: "<<ex.what());
				exit(1);
			}
		}
		else if (pidPcd_ < 0)            // failed to fork
		{
			LOG(ERROR, "Failed to fork!");
			throw std::runtime_error ("fork failed: " + std::string(strerror(errno)));
		}
		else  // parent
		{
			// Code only executed by parent process
			sleep(2);
		}

		return rc;
	}

	void shutdownNrePcd()
	{
		int c;
		int nStatus;
		if (pidPcd_ <= 1)
		{
			LOG(ERROR, "cannot send TERM signal to child with pid: " << pidPcd_);
			throw std::runtime_error ("pcd does not have a valid pid (<= 1)");
		}

		kill(pidPcd_, SIGTERM);

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

	int m_nITER;
	int m_sleep_interval ;
    std::string m_strWorkflow;

	fhg::com::io_service_pool *m_ptrPool;
	fhg::com::kvs::server::kvsd *m_ptrKvsd;
	fhg::com::tcp_server *m_ptrServ;
	boost::thread *m_ptrThrd;

	pid_t pidPcd_;
};


void MyFixture::startDaemons(const std::string& workerUrl)
{
	string strAppGuiUrl   	= "";
	string strLogGuiUrl   	= "";

	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	//LOG( DEBUG, "Create the Orchestrator ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<RealWorkflowEngine>::create("orchestrator_0", addrOrch);
	ptrOrch->start_agent();

	//LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	ptrAgg->start_agent();

	// use external scheduler and real GWES
	//LOG( DEBUG, "Create the NRE ...");
	sdpa::daemon::NRE<WorkerClient>::ptr_t
		ptrNRE = sdpa::daemon::NREFactory<RealWorkflowEngine, WorkerClient>::create("NRE",
											 addrNRE,"aggregator_0", workerUrl, strLogGuiUrl );

	try {
		ptrNRE->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		ptrOrch->shutdown();
		ptrAgg->shutdown();
		ptrNRE->shutdown();

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
		sdpa::job_id_t job_id_user; int nTrials = 0;;
retry:	try {
			nTrials++; job_id_user = ptrCli->submitJob(m_strWorkflow);
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			if(nTrials > NMAXTRIALS)
			{
				LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

				ptrNRE->shutdown();
				ptrAgg->shutdown();
				ptrOrch->shutdown();
				ptrCli->shutdown_network();
				ptrCli.reset();
				return;
			}
			else
				goto retry;

		}

		LOG( DEBUG, "*****JOB #"<<k<<"******");

		std::string job_status =  ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = ptrCli->queryJob(job_id_user);
			LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

			boost::this_thread::sleep(boost::posix_time::microseconds(m_sleep_interval));
		}

		LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
		ptrCli->retrieveResults(job_id_user);

		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
	}

	ptrOrch->shutdown();
	ptrAgg->shutdown();
	ptrNRE->shutdown();

	ptrCli->shutdown_network();
	ptrCli.reset();
}

BOOST_FIXTURE_TEST_SUITE( test_suite_agent_int, MyFixture )

BOOST_AUTO_TEST_CASE( testActivityRealWeAllCompAndNreWorkerSpawnedByNRE )
{
	LOG( DEBUG, "***** testActivityRealWeAllCompAndNreWorkerSpawnedByNRE *****"<<std::endl);
	string strLogGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	//LOG( DEBUG, "Create the Orchestrator ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<RealWorkflowEngine>::create("orchestrator_0", addrOrch);
	ptrOrch->start_agent();

	//LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	ptrAgg->start_agent();

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	// use external scheduler and real GWES
	//LOG( DEBUG, "Create the NRE ...");
	sdpa::daemon::NRE<WorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NREFactory<RealWorkflowEngine, WorkerClient>::create("NRE_0",
				                             addrNRE,"aggregator_0",
				                             workerUrl,
				                             strLogGuiUrl,
				                             bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		ptrNRE_0->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		ptrNRE_0->shutdown();
		ptrAgg->shutdown();
		ptrOrch->shutdown();

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
		//sdpa::job_id_t job_id_user = ptrCli->submitJob(m_strWorkflow);
		sdpa::job_id_t job_id_user; int nTrials = 0;;
retry:	try {
			nTrials++; job_id_user = ptrCli->submitJob(m_strWorkflow);
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			if(nTrials > NMAXTRIALS)
			{
				LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

				ptrNRE_0->shutdown();
				ptrAgg->shutdown();
				ptrOrch->shutdown();
				ptrCli->shutdown_network();
				ptrCli.reset();
				return;
			}
			else
				goto retry;

		}

		LOG( DEBUG, "*****JOB #"<<k<<"******");

		std::string job_status = ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = ptrCli->queryJob(job_id_user);
			LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

			boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
		}

		LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
		ptrCli->retrieveResults(job_id_user);

		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
	}

	LOG(WARN, "NRE_0 refcount = "<<ptrNRE_0.use_count());
	ptrNRE_0->shutdown();

	LOG(WARN, "aggregator_0 refcount = "<<ptrAgg.use_count());
	ptrAgg->shutdown();

	LOG(WARN, "orchestrator_0 refcount = "<<ptrOrch.use_count());
	ptrOrch->shutdown();

	ptrCli->shutdown_network();
    ptrCli.reset();

	LOG( DEBUG, "The test case testActivityRealWeAllCompAndNreWorkerSpywnedByNRE terminated!");
}

BOOST_AUTO_TEST_CASE( testActivityRealWeAllCompAndNreWorkerSpawnedByTest )
{
	LOG( DEBUG, "***** testActivityRealWeAllCompAndNreWorkerSpawnedByTest *****"<<std::endl);

	string workerUrl = "127.0.0.1:12500";

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back( TESTS_EXAMPLE_STRESSTEST_MODULES_PATH );

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back( TESTS_FVM_PC_FAKE_MODULE );

	try {
		startNrePcd( workerUrl, TESTS_NRE_PCD_BIN_PATH, v_fake_PC_search_path, v_module_preload );
	}
	catch(const std::exception& ex)
	{
		LOG( DEBUG, "Could not start the process container daemon!");
		throw;
	}

	startDaemons(workerUrl);
	shutdownNrePcd();

	LOG( DEBUG, "The test case testActivityRealWeAllCompAndNreWorkerSpawnedByTest terminated!");
}

BOOST_AUTO_TEST_CASE( testActivityRealWeAllCompActExec )
{
	LOG( DEBUG, "***** testActivityRealWeAllCompAndActExec *****"<<std::endl);
	string strLogGuiUrl   = "";
	string workerUrl = "127.0.0.1:12500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	//LOG( DEBUG, "Create the Orchestrator ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<RealWorkflowEngine>::create("orchestrator_0", addrOrch);
	ptrOrch->start_agent();

	//LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	ptrAgg->start_agent();

	// use external scheduler and dummy WE
	//LOG( DEBUG, "Create the NRE ...");
	sdpa::daemon::NRE<WorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NREFactory<RealWorkflowEngine, WorkerClient>::create("NRE_0", addrNRE,"aggregator_0", workerUrl, strLogGuiUrl );

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
		ptrNRE_0->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		ptrOrch->shutdown();
		ptrAgg->shutdown();
		ptrNRE_0->shutdown();

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
		//sdpa::job_id_t job_id_user = ptrCli->submitJob(m_strWorkflow);
		sdpa::job_id_t job_id_user; int nTrials = 0;;
retry:	try {
			nTrials++; job_id_user = ptrCli->submitJob(m_strWorkflow);
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			if(nTrials > NMAXTRIALS)
			{
				LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

				ptrNRE_0->shutdown();
				ptrAgg->shutdown();
				ptrOrch->shutdown();
				ptrCli->shutdown_network();
				ptrCli.reset();
				return;
			}
			else
				goto retry;

		}

		LOG( DEBUG, "*****JOB #"<<k<<"******");

		std::string job_status =  ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = ptrCli->queryJob(job_id_user);
			LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

			boost::this_thread::sleep(boost::posix_time::microseconds(m_sleep_interval));
		}

		LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
		ptrCli->retrieveResults(job_id_user);

		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
	}

	// process container terminates ...
	LOG( INFO, "terminating...");
	if (! executor->stop())
		LOG( WARN, "executor did not stop correctly...");

	ptrNRE_0->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();
	ptrCli->shutdown_network();
	ptrCli.reset();

	LOG( DEBUG, "The test case testActivityRealWeAllCompAndActExec with fvm-pc terminated!");
}

BOOST_AUTO_TEST_CASE( testActivityDummyWeAllCompActExec )
{
	LOG( DEBUG, "***** testActivityDummyWeAllCompAndNreWorker *****"<<std::endl);
	string strAppGuiUrl   = "";
	string strLogGuiUrl   = "";
	string workerUrl = "127.0.0.1:12500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	//LOG( DEBUG, "Create the Orchestrator ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create("orchestrator_0", addrOrch, strAppGuiUrl);
	ptrOrch->start_agent();

	//LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<DummyWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	ptrAgg->start_agent();

	// use external scheduler and dummy WE
	//LOG( DEBUG, "Create the NRE ...");
	sdpa::daemon::NRE<WorkerClient>::ptr_t
		ptrNRE_0 = sdpa::daemon::NREFactory<DummyWorkflowEngine, WorkerClient>::create("NRE_0", addrNRE,"aggregator_0", workerUrl,  strAppGuiUrl, strLogGuiUrl );

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
		ptrNRE_0->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		LOG( WARN, "TODO: implement NRE-PCD fork/exec with a RestartStrategy->restart()");

		ptrOrch->shutdown();
		ptrAgg->shutdown();
		ptrNRE_0->shutdown();

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
		//sdpa::job_id_t job_id_user = ptrCli->submitJob(m_strWorkflow);
		sdpa::job_id_t job_id_user; int nTrials = 0;;
retry:	try {
			nTrials++; job_id_user = ptrCli->submitJob(m_strWorkflow);
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			if(nTrials > NMAXTRIALS)
			{
				LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

				ptrNRE_0->shutdown();
				ptrAgg->shutdown();
				ptrOrch->shutdown();
				ptrCli->shutdown_network();
				ptrCli.reset();
				return;
			}
			else
				goto retry;

		}

		LOG( DEBUG, "*****JOB #"<<k<<"******");

		std::string job_status =  ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			job_status = ptrCli->queryJob(job_id_user);
			LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

			boost::this_thread::sleep(boost::posix_time::microseconds(m_sleep_interval));
		}

		LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
		ptrCli->retrieveResults(job_id_user);

		LOG( DEBUG, "User: delete the job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
	}

	ptrNRE_0->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "terminating...");
	if (! executor->stop())
		LOG( WARN, "executor did not stop correctly...");

	ptrCli->shutdown_network();
	ptrCli.reset();

	LOG( DEBUG, "The test case testActivityDummyWeAllCompAndNreWorker terminated!");
}

BOOST_AUTO_TEST_SUITE_END()
