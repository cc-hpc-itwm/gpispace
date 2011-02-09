#define BOOST_TEST_MODULE TestTimedoutWorkers
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
#include <sys/wait.h>

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

const int NMAXTRIALS = 10000;
const int NMAXTHRDS = 10;

namespace po = boost::program_options;

#define NO_GUI ""
typedef we::mgmt::layer<id_type, we::activity_t> RealWorkflowEngine;
typedef sdpa::nre::worker::NreWorkerClient WorkerClient;

namespace sdpa { namespace tests { namespace worker {
  class NreWorkerClient
  {
  public:
    explicit
    NreWorkerClient( const std::string &nre_worker_location
                      // TODO: fixme, this is ugly
                      , bool bLaunchNrePcd = false
                      , const std::string & fvmPCBinary = ""
                      , const std::vector<std::string>& fvmPCSearchPath = std::vector<std::string>()
                      , const std::vector<std::string>& fvmPCPreLoad = std::vector<std::string>()
                      )
    : SDPA_INIT_LOGGER("TestNreWorkerClient") { }

    void set_ping_interval(unsigned long /*seconds*/){}
    void set_ping_timeout(unsigned long /*seconds*/){}
    void set_ping_trials(std::size_t /*max_tries*/){}

    void set_location(const std::string &str_loc){ nre_worker_location_ = str_loc; }

    unsigned int start() throw (std::exception) { LOG( INFO, "Start the test NreWorkerClient ..."); return 0;}
    void stop() throw (std::exception) { LOG( INFO, "Stop the test NreWorkerClient ...");}

    void cancel() throw (std::exception){ throw std::runtime_error("not implemented"); }

    sdpa::nre::worker::execution_result_t execute(const encoded_type& in_activity, unsigned long walltime = 0) throw (sdpa::nre::worker::WalltimeExceeded, std::exception)
	{
    	LOG( INFO, "Executing activity ...");
    	sleep(1);
    	LOG( INFO, "Report activity finished ...");

    	sdpa::nre::worker::execution_result_t exec_res(std::make_pair(sdpa::nre::worker::ACTIVITY_FINISHED, "empty result"));
    	return exec_res;
	}

  private:
    std::string nre_worker_location_;
    SDPA_DECLARE_LOGGER();
  };
}}}

typedef sdpa::tests::worker::NreWorkerClient TestWorkerClient;

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
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

		//seda::StageRegistry::instance().clear();

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

	void run_client(int i);

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

void MyFixture::run_client(int i)
{
	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	config.parse_command_line(cav);

	std::ostringstream oss;
	oss<<"client_"<<i;

	std::string outstage("sdpa.apps.");
	outstage += oss.str();
	outstage += (".out");

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config, oss.str(), outstage );
	ptrCli->configure_network( config );


	for( int k=0; k<m_nITER; k++ )
	{
		int nTrials = 0;
		sdpa::job_id_t job_id_user;

		try {

			//LOG( DEBUG, "Submitting the following test workflow: \n"<<m_strWorkflow);
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
				usleep(5*m_sleep_interval);
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

				sleep(5);
			}
		}

		nTrials = 0;

		try {
				LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
				ptrCli->retrieveResults(job_id_user);
				usleep(5*m_sleep_interval);
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

			sleep(5);
		}

		nTrials = 0;

		try {
			LOG( DEBUG, "User: delete the job "<<job_id_user);
			ptrCli->deleteJob(job_id_user);
			usleep(5*m_sleep_interval);
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

			sleep(5);
		}
	}

	ptrCli->shutdown_network();
    ptrCli.reset();
}

BOOST_FIXTURE_TEST_SUITE( test_StopRestartAgents, MyFixture );


BOOST_AUTO_TEST_CASE( testConcurrentClients )
{
	LOG( INFO, "***** testJobSubmToWeFails *****"<<std::endl);

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

	boost::thread* arrThreadClient = new boost::thread[NMAXTHRDS];

	for(int i=0;i<NMAXTHRDS;i++)
		arrThreadClient[i] = boost::thread(boost::bind(&MyFixture::run_client, this, i));
	sleep(5);

	for(int i=0;i<NMAXTHRDS;i++)
	{
		arrThreadClient[i].join();
		LOG( INFO, "The client thread "<<i<<" joined the main thread°!" );
	}

	sleep(1);

	ptrNRE->shutdown();
	ptrAgg->shutdown();
	ptrOrch->shutdown();

	LOG( INFO, "The test case testJobSubmToWeFails terminated!" );
}


BOOST_AUTO_TEST_SUITE_END()
