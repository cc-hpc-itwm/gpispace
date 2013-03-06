#define BOOST_TEST_MODULE TestMultipleMasters
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
#include <boost/filesystem/path.hpp>

#include <sdpa/engine/EmptyWorkflowEngine.hpp>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>

using namespace sdpa::tests;
using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("0"); return s; }

const int NMAXTRIALS = 10000;
const int NMAXTHRDS = 3;
const int MAX_CAP = 100;

namespace po = boost::program_options;
#define NO_GUI ""

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

		m_serv->stop ();
		m_pool->stop ();
		m_thrd->join ();

		delete m_thrd;
		delete m_serv;
		delete m_kvsd;
		delete m_pool;

		seda::StageRegistry::instance().clear();
	}

	void run_client(int i);
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

	std::stringstream sstrOrch;
	std::stringstream sstrAgg;

	sdpa::master_info_list_t m_arrAggMasterInfo;

	boost::thread m_threadClient;
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

				boost::this_thread::sleep(boost::posix_time::seconds(3));
			}
		}

		nTrials = 0;

		try {
				LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
				ptrCli->retrieveResults(job_id_user);
				boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			//if(nTrials++ > NMAXTRIALS)
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
			boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			//if(nTrials++ > NMAXTRIALS)
			{
				LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

				ptrCli->shutdown_network();
				boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
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
//	kernel->load_plugin (TESTS_GUI_PLUGIN_PATH);
	kernel->load_plugin (TESTS_WFE_PLUGIN_PATH);
	kernel->load_plugin (TESTS_FVM_FAKE_PLUGIN_PATH);
	kernel->load_plugin (TESTS_DRTS_PLUGIN_PATH);

	return kernel;
}

BOOST_FIXTURE_TEST_SUITE( testMultipleMastersSuite, MyFixture );


BOOST_AUTO_TEST_CASE( testMultipleMastersEmptyWEPush )
{
	LOG( INFO, "testMultipleMastersEmptyWEPush");

	//string strAppGuiUrl   	= "";
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgg_0 	= "127.0.0.1:7700";
	string addrAgg_1 	= "127.0.0.1:7701";
	string addrAgg_00 	= "127.0.0.1:7702";

	LOG( INFO, "Create Orchestrator with an empty workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	LOG( INFO, "Create the Agent ...");
	sdpa::daemon::Agent::ptr_t ptrAgent0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgg_0,m_arrAggMasterInfo, 100);
	ptrAgent0->start_agent(false);

	LOG( INFO, "Create the Agent ...");
	sdpa::daemon::Agent::ptr_t ptrAgent1 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_1", addrAgg_1, m_arrAggMasterInfo, 100);
	ptrAgent1->start_agent(false);

	LOG( INFO, "Create the Agent ...");
	sdpa::master_info_list_t arrAgent00MasterInfo;
	sdpa::daemon::Agent::ptr_t ptrAgent00 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_00", addrAgg_00, arrAgent00MasterInfo, 100);
	ptrAgent00->start_agent(false);

	ptrAgent00->addMaster("agent_0");
	ptrAgent00->addMaster("agent_1");

	sdpa::shared_ptr<fhg::core::kernel_t> drts_00( create_drts("drts_00", "agent_00") );
	boost::thread drts_00_thread = boost::thread(&fhg::core::kernel_t::run, drts_00);

	boost::thread* arrThreadClient = new boost::thread[NMAXTHRDS];

	for(int i=0;i<NMAXTHRDS;i++)
		arrThreadClient[i] = boost::thread(boost::bind(&MyFixture::run_client, this, i));

	boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));

	for(int i=0;i<NMAXTHRDS;i++)
	{
		if( arrThreadClient[i].joinable() )
			arrThreadClient[i].join();

		LOG( INFO, "The client thread "<<i<<" joined the main thread°!" );
	}

	drts_00->stop();
	if(drts_00_thread.joinable())
		drts_00_thread.join();

	ptrAgent00->shutdown();
	ptrAgent0->shutdown();
	ptrAgent1->shutdown();

	ptrOrch->shutdown();

	LOG( INFO, "The test case testMultipleMastersEmptyWEPush terminated!" );
}

BOOST_AUTO_TEST_SUITE_END()
