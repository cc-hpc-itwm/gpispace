#define BOOST_TEST_MODULE TestStopRestartAgents
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
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
#include <sdpa/daemon/aggregator/AggregatorFactory.hpp>
#include <sdpa/daemon/nre/NREFactory.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/nre/SchedulerNRE.hpp>

#include <sdpa/client/ClientApi.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/Strategy.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>
#include <boost/thread.hpp>

#include "tests_config.hpp"
#include <boost/filesystem/path.hpp>

#include <sdpa/engine/DummyWorkflowEngine.hpp>
#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include <sdpa/engine/RealWorkflowEngine.hpp>

using namespace sdpa::tests;
using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("0"); return s; }

const int NMAXTRIALS = 3;

namespace po = boost::program_options;

#define NO_GUI ""

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
			, m_sleep_interval(1000) //microseconds
			, m_pool (0)
	    	, m_kvsd (0)
	    	, m_serv (0)
	    	, m_thrd (0)
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

		sstrOrch.str("");
		sstrAgg.str("");
		sstrNRE.str("");

		m_pool->stop ();
		m_thrd->join ();
		delete m_thrd;
		delete m_serv;
		delete m_kvsd;
		delete m_pool;
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

	std::stringstream sstrOrch;
	std::stringstream sstrAgg;
	std::stringstream sstrNRE;

	boost::thread m_threadClient;
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

				boost::this_thread::sleep(boost::posix_time::seconds(3));
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
				LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

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
				LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

				ptrCli->shutdown_network();
				ptrCli.reset();
				return;
			}

			boost::this_thread::sleep(boost::posix_time::seconds(3));
		}
	}

	ptrCli->shutdown_network();
    ptrCli.reset();
}

BOOST_FIXTURE_TEST_SUITE( test_StopRestartAgents, MyFixture );

BOOST_AUTO_TEST_CASE( testStopRestartOrch )
{
	LOG( INFO, "***** testStopRestartOrch *****"<<std::endl);

	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	bool bLaunchNrePcd = true;

	LOG( INFO, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);
	ptrOrch->start_agent();

	LOG( INFO, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	ptrAgg->start_agent();

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	LOG( INFO, "Create the NRE ...");
	sdpa::daemon::NRE<WorkerClient>::ptr_t
		ptrNRE = sdpa::daemon::NREFactory<RealWorkflowEngine, WorkerClient>::create("NRE_0",
				                             addrNRE,"aggregator_0",
				                             workerUrl,
				                             strGuiUrl,
				                             bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		ptrNRE->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		return;
	}

	LOG( INFO, "Deliberately shutdown the orchestrator now!");
	ptrOrch->shutdown(sstrOrch);
	//usleep(5);
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	LOG( INFO, "Start a new instance of the orchestrator now!");
	ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);
	ptrOrch->start_agent(sstrOrch);

	// give some time to the aggregator to re-register
	while(!ptrAgg->is_registered())
		boost::this_thread::sleep(boost::posix_time::seconds(1));

	LOG( INFO, "Shutdown the orchestrator, the aggregator and the nre!");
	ptrNRE->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();
	LOG( INFO, "The test case testStopRestartOrch terminated!");
}

BOOST_AUTO_TEST_CASE( testStopRestartAgg )
{
	LOG( INFO, "***** testStopRestartAgg *****"<<std::endl);

	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	bool bLaunchNrePcd = true;

	LOG( INFO, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);
	ptrOrch->start_agent();

	LOG( INFO, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	ptrAgg->start_agent();

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	LOG( INFO, "Create the NRE ...");
	sdpa::daemon::NRE<WorkerClient>::ptr_t
		ptrNRE = sdpa::daemon::NREFactory<RealWorkflowEngine, WorkerClient>::create("NRE_0",
				                             addrNRE,"aggregator_0",
				                             workerUrl,
				                             strGuiUrl,
				                             bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		ptrNRE->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		return;
	}

	LOG( INFO, "Deliberately shutdown the aggregator now!");
	ptrAgg->shutdown(sstrOrch);
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	LOG( INFO, "Start a new instance of the aggregator now!");
	ptrAgg = sdpa::daemon::AggregatorFactory<void>::create("aggregator_0", addrAgg, "orchestrator_0");
	ptrAgg->start_agent(sstrOrch);

	// give some time to the NRE to re-register
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	ptrNRE->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case testStopRestartAgg terminated!");
	LOG( INFO, "Shutdown the orchestrator, the aggregator and the nre!");
}

BOOST_AUTO_TEST_CASE( testBackupRecoverOrchNoWfeWithClient )
{
	LOG( INFO, "***** testBackupRecoverOrchNoWfeWithClient *****"<<std::endl);

	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	bool bLaunchNrePcd = false;
	//typedef sdpa::nre::worker::NreWorkerClient WorkerClient;

	LOG( INFO, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);
	ptrOrch->start_agent();

	LOG( INFO, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<void>::create("aggregator_0", addrAgg,"orchestrator_0");
	ptrAgg->start_agent();

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	LOG( INFO, "Create the NRE ...");
	sdpa::daemon::NRE<TestWorkerClient>::ptr_t
		ptrNRE = sdpa::daemon::NREFactory<void, TestWorkerClient>::create("NRE_0",
											 addrNRE,"aggregator_0",
											 workerUrl,
											 strGuiUrl,
											 bLaunchNrePcd,
											 TESTS_NRE_PCD_BIN_PATH,
											 v_fake_PC_search_path,
											 v_module_preload );

	try {
		ptrNRE->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		return;
	}

	m_threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

	boost::this_thread::sleep(boost::posix_time::seconds(1));

	LOG( DEBUG, "Shutdown the orchestrator");

	ptrOrch->shutdown(sstrOrch);

	boost::this_thread::sleep(boost::posix_time::seconds(3));

	// now try to recover the system
	sdpa::daemon::Orchestrator::ptr_t ptrRecOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);
	//ptrRecOrch->recover(filename);

	LOG( DEBUG, "Re-start the orchestrator");
	ptrRecOrch->start_agent(sstrOrch);

	// give some time to the NRE to re-register
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	m_threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	ptrNRE->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case testBackupRecoverOrch2 terminated!" );
}

BOOST_AUTO_TEST_CASE( testBackupRecoverOrchEmptyWfeWithClient )
{
	LOG( INFO, "***** testBackupRecoverOrchEmptyWfeWithClient *****"<<std::endl);

	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	bool bLaunchNrePcd = false;

	LOG( INFO, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);
	ptrOrch->start_agent();

	LOG( INFO, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<EmptyWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	ptrAgg->start_agent();

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	LOG( INFO, "Create the NRE ...");
	sdpa::daemon::NRE<TestWorkerClient>::ptr_t
		ptrNRE = sdpa::daemon::NREFactory<EmptyWorkflowEngine, TestWorkerClient>::create("NRE_0",
											 addrNRE,"aggregator_0",
											 workerUrl,
											 strGuiUrl,
											 bLaunchNrePcd,
											 TESTS_NRE_PCD_BIN_PATH,
											 v_fake_PC_search_path,
											 v_module_preload );

	try {
		ptrNRE->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		return;
	}

	m_threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

	boost::this_thread::sleep(boost::posix_time::seconds(1));

	LOG( DEBUG, "Shutdown the orchestrator");
	ptrOrch->shutdown(sstrOrch);

	//LOG( DEBUG, "After shutdown the content of osstrOrch is: \n"<<sstrOrch.str() );
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	// now try to recover the system
	sdpa::daemon::Orchestrator::ptr_t ptrRecOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);

	LOG( DEBUG, "Re-start the orchestrator");
	ptrRecOrch->start_agent(sstrOrch);

	// give some time to the NRE to re-register
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	m_threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	ptrNRE->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case testBackupRecoverOrch2 terminated!" );
}

BOOST_AUTO_TEST_CASE( testBackupRecoverOrchDummyWfeWithClient )
{
	LOG( INFO, "***** testBackupRecoverOrchDummyWfeWithClient *****"<<std::endl);

	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	bool bLaunchNrePcd = false;

	LOG( INFO, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);
	ptrOrch->start_agent();

	LOG( INFO, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<DummyWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	ptrAgg->start_agent();

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	LOG( INFO, "Create the NRE ...");
	sdpa::daemon::NRE<TestWorkerClient>::ptr_t
		ptrNRE = sdpa::daemon::NREFactory<DummyWorkflowEngine, TestWorkerClient>::create("NRE_0",
											 addrNRE,"aggregator_0",
											 workerUrl,
											 strGuiUrl,
											 bLaunchNrePcd,
											 TESTS_NRE_PCD_BIN_PATH,
											 v_fake_PC_search_path,
											 v_module_preload );

	try {
		ptrNRE->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		return;
	}

	m_threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));

	boost::this_thread::sleep(boost::posix_time::seconds(1));


	LOG( DEBUG, "Shutdown the orchestrator");
	ptrOrch->shutdown(sstrOrch);


	LOG( DEBUG, "After shutdown the content of osstrOrch is: \n"<<sstrOrch.str() );

	boost::this_thread::sleep(boost::posix_time::seconds(3));

	// now try to recover the system
	sdpa::daemon::Orchestrator::ptr_t ptrRecOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);

	LOG( DEBUG, "Re-start the orchestrator");
	ptrRecOrch->start_agent(sstrOrch);

	// give some time to the NRE to re-register
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	m_threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	ptrNRE->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case testBackupRecoverOrch2 terminated!" );
}

BOOST_AUTO_TEST_CASE( testBackupRecoverOrchRealWfeWithClient )
{
	LOG( INFO, "***** testBackupRecoverOrchRealWfeWithClient *****"<<std::endl);

	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

#ifdef USE_REAL_WE
	bool bLaunchNrePcd = true;
#else
	bool bLaunchNrePcd = false;
#endif

	LOG( INFO, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);
	ptrOrch->start_agent();

	LOG( INFO, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	ptrAgg->start_agent();

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	LOG( INFO, "Create the NRE ...");
	sdpa::daemon::NRE<WorkerClient>::ptr_t
		ptrNRE = sdpa::daemon::NREFactory<RealWorkflowEngine, WorkerClient>::create("NRE_0",
											 addrNRE,"aggregator_0",
											 workerUrl,
											 strGuiUrl,
											 bLaunchNrePcd,
											 TESTS_NRE_PCD_BIN_PATH,
											 v_fake_PC_search_path,
											 v_module_preload );

	try {
		ptrNRE->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		return;
	}

	m_threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	LOG( DEBUG, "Shutdown the orchestrator");
	ptrOrch->shutdown(sstrOrch);
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	// now try to recover the system
	sdpa::daemon::Orchestrator::ptr_t ptrRecOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);

	LOG( DEBUG, "Re-start the orchestrator");
	ptrRecOrch->start_agent(sstrOrch);

	// give some time to the NRE to re-register
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	m_threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	ptrNRE->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case testBackupRecoverOrchRealWfeWithClient terminated!" );
}

BOOST_AUTO_TEST_CASE( testStopRestartAggRealWeWithClient )
{
	LOG( INFO, "***** testStopRestartAggRealWeWithClient *****"<<std::endl);

	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

#ifdef USE_REAL_WE
	bool bLaunchNrePcd = true;
#else
	bool bLaunchNrePcd = false;
#endif

	LOG( INFO, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);
	ptrOrch->start_agent();

	LOG( INFO, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	ptrAgg->start_agent();

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	LOG( INFO, "Create the NRE ...");
	sdpa::daemon::NRE<WorkerClient>::ptr_t
		ptrNRE = sdpa::daemon::NREFactory<RealWorkflowEngine, WorkerClient>::create("NRE_0",
				                             addrNRE,"aggregator_0",
				                             workerUrl,
				                             strGuiUrl,
				                             bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		ptrNRE->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		return;
	}

	m_threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	LOG( INFO, "Deliberately shutdown the aggregator now!");
	ptrAgg->shutdown(sstrAgg);
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	LOG( INFO, "Start a new instance of the aggregator now!");
	ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg, "orchestrator_0");
	ptrAgg->start_agent(sstrAgg);

	// give some time to the NRE to re-register
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	m_threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	ptrNRE->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case testStopRestartAggRealWeWithClient terminated!");
}

BOOST_AUTO_TEST_CASE( testStopRestartNRERealWeWithClient )
{
	LOG( INFO, "***** testStopRestartNRERealWeWithClient *****"<<std::endl);

	string strGuiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgg 		= "127.0.0.1";
	string addrNRE 		= "127.0.0.1";

#ifdef USE_REAL_WE
	bool bLaunchNrePcd = true;
#else
	bool bLaunchNrePcd = false;
#endif

	LOG( INFO, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);
	ptrOrch->start_agent();

	LOG( INFO, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	ptrAgg->start_agent();

	std::vector<std::string> v_fake_PC_search_path;
	v_fake_PC_search_path.push_back(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH);

	std::vector<std::string> v_module_preload;
	v_module_preload.push_back(TESTS_FVM_PC_FAKE_MODULE);

	LOG( INFO, "Create the NRE ...");
	sdpa::daemon::NRE<WorkerClient>::ptr_t
		ptrNRE = sdpa::daemon::NREFactory<RealWorkflowEngine, WorkerClient>::create( "NRE_0",
																					 addrNRE,
																					 "aggregator_0",
																					 workerUrl,
																					 strGuiUrl,
																					 bLaunchNrePcd,
																					 TESTS_NRE_PCD_BIN_PATH,
																					 v_fake_PC_search_path,
																					 v_module_preload );

	try {
		ptrNRE->start_agent();
	}
	catch (const std::exception &ex) {
		LOG( FATAL, "Could not start NRE: " << ex.what());
		return;
	}

	m_threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	LOG( INFO, "Deliberately shutdown the NRE now!");
	ptrNRE->shutdown(sstrNRE);

	LOG( DEBUG, "After shutdown the content of sstrNRE is: \n"<<sstrNRE.str() );

	boost::this_thread::sleep(boost::posix_time::seconds(3));

	LOG( INFO, "Start a new instance of the NRE now!");
	ptrNRE = sdpa::daemon::NREFactory<RealWorkflowEngine, WorkerClient>::create( "NRE_0",
																				 addrNRE,
																				 "aggregator_0",
																				 workerUrl,
																				 strGuiUrl,
																				 bLaunchNrePcd,
																				 TESTS_NRE_PCD_BIN_PATH,
																				 v_fake_PC_search_path,
																				 v_module_preload );

	ptrNRE->start_agent(sstrNRE);
	// give some time to the NRE to re-register
	boost::this_thread::sleep(boost::posix_time::seconds(3));

	m_threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	ptrNRE->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case testStopRestartNRE terminated!");
	LOG( INFO, "Shutdown the orchestrator, the aggregator and the nre!");
}

BOOST_AUTO_TEST_SUITE_END()
