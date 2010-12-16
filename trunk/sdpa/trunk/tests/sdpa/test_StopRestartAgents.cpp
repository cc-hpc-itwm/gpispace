#define BOOST_TEST_MODULE TestAgentsSerialization
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
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <sdpa/daemon/JobManager.hpp>

#include <boost/serialization/export.hpp>
#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/orchestrator/SchedulerOrch.hpp>
#include <sdpa/daemon/aggregator/AggregatorFactory.hpp>
#include <sdpa/daemon/nre/NREFactory.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <sdpa/daemon/nre/SchedulerNRE.hpp>
#include <tests/sdpa/DummyWorkflowEngine.hpp>

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
#include <sys/wait.h>

#include <sdpa/daemon/EmptyWorkflowEngine.hpp>
#include <we/mgmt/layer.hpp>

using namespace sdpa::tests;
using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("12100"); return s; }
const int NMAXTRIALS=3;

namespace po = boost::program_options;

#define NO_GUI ""
typedef we::mgmt::layer<id_type, we::activity_t> RealWorkflowEngine;

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

		//seda::StageRegistry::instance().clear();

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

	int m_nITER;
	int m_sleep_interval ;
    std::string m_strWorkflow;

	fhg::com::io_service_pool *m_ptrPool;
	fhg::com::kvs::server::kvsd *m_ptrKvsd;
	fhg::com::tcp_server *m_ptrServ;
	boost::thread *m_ptrThrd;

	pid_t pidPcd_;
};

BOOST_FIXTURE_TEST_SUITE( test_StopRestartAgents, MyFixture );

BOOST_AUTO_TEST_CASE( testStopRestartOrchWithNoWe )
{
	LOG( DEBUG, "***** testOrchestratorNoWe *****"<<std::endl);
	string strGuiUrl   = "";
	string workerUrl = "127.0.0.1:5500";
	string addrOrch = "127.0.0.1";
	string addrAgg = "127.0.0.1";
	string addrNRE = "127.0.0.1";

	typedef void OrchWorkflowEngine;
	typedef sdpa::nre::worker::NreWorkerClient WorkerClient;

	bool bLaunchNrePcd = true;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	//LOG( DEBUG, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);
	ptrOrch->start();

	//LOG( DEBUG, "Create the Aggregator ...");
	sdpa::daemon::Aggregator::ptr_t ptrAgg = sdpa::daemon::AggregatorFactory<RealWorkflowEngine>::create("aggregator_0", addrAgg,"orchestrator_0");
	ptrAgg->start();

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
				                             strGuiUrl,
				                             bLaunchNrePcd,
				                             TESTS_NRE_PCD_BIN_PATH,
				                             v_fake_PC_search_path,
				                             v_module_preload );

	try {
		ptrNRE_0->start();
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
		sdpa::job_id_t job_id_user; int nTrials = 0;;
retry:	try {
			nTrials++;
			job_id_user = ptrCli->submitJob(m_strWorkflow);
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

		// stop re-start
		if(k==1)
		{
			ptrOrch->shutdown();
			usleep(5*m_sleep_interval);
			ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);
			ptrOrch->start();

			ptrCli->shutdown_network();
		    ptrCli.reset();

		    ptrCli = sdpa::client::ClientApi::create( config );
		    ptrCli->configure_network( config );
		}

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

	ptrNRE_0->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	ptrCli->shutdown_network();
    ptrCli.reset();

	LOG( DEBUG, "The test case testOrchestratorNoWe terminated!");
}

/*
BOOST_AUTO_TEST_CASE( testBackupRecoverOrch )
{
	string addrOrch = "127.0.0.1";

	std::cout<<std::endl<<"----------------Begin  testBackupRecoverOrch----------------"<<std::endl;
	std::string filename = "testBackupRecover.txt"; // = boost::archive::tmpdir());filename += "/testfile";

	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config );
	ptrCli->configure_network( config );

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	LOG( DEBUG, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch);
	ptrOrch->start();

	LOG( DEBUG, "Submit 5 jobs to the orchestrator ...");
	for(int k=0; k<5; k++ )
		sdpa::job_id_t job_id_user = ptrCli->submitJob(m_strWorkflow);

	ptrOrch->print();
	LOG( DEBUG, "Bakcup the orchestrator ino the file "<<filename);
	ptrOrch->backup(filename);
	ptrOrch->shutdown();


	sleep(1);

	ptrCli->shutdown_network();
	ptrCli.reset();

	//seda::StageRegistry::instance().clear();

	// now try to recover the system
	sdpa::daemon::Orchestrator::ptr_t ptrRecOrch = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create("orchestrator_0", "127.0.0.1:7000");
	ptrRecOrch->recover(filename);
	ptrRecOrch->start();

	sleep(1);

	ptrRecOrch->shutdown();
	sleep(1);

	std::cout<<std::endl<<"----------------End  testBackupRecoverOrch----------------"<<std::endl;
}
*/

BOOST_AUTO_TEST_SUITE_END()
