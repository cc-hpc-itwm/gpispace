///
 // =====================================================================================
 //
 //       Filename:  test_Components.cpp
 //
 //    Description:  test all components, each with a real gwes, using a real user client
 //
 //        Version:  1.0
 //        Created:
 //       Revision:  none
 //       Compiler:  gcc
 //
 //         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 //        Company:  Fraunhofer ITWM
 //
 // =====================================================================================
 ///
#define BOOST_TEST_MODULE TestTopologies
#include "sdpa/daemon/JobFSM.hpp"
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

#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <seda/StageRegistry.hpp>

#include <boost/filesystem/path.hpp>

#include <sdpa/engine/DummyWorkflowEngine.hpp>
#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include <sdpa/engine/RealWorkflowEngine.hpp>

#include <boost/thread.hpp>
#include <boost/shared_array.hpp>
#include <sdpa/types.hpp>

//plugin
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>

const int NMAXTRIALS=5;
const int MAX_CAP = 100;
const int NAGENTS = 1;

const int BUNCH_SIZE = 1;

namespace po = boost::program_options;

using namespace std;
using namespace sdpa::tests;

#define NO_GUI ""

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("0"); return s; }

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
			, m_pool (0)
	    	, m_kvsd (0)
	    	, m_serv (0)
	    	, m_thrd (0)
			, m_arrAgentMasterInfo(1, MasterInfo("orchestrator_0"))
	{
		//initialize and start_agent the finite state machine

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
		sstrAgent.str("");

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
	void run_cannon_client();

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

	sdpa::master_info_list_t m_arrAgentMasterInfo;

	std::stringstream sstrOrch;
	std::stringstream sstrAgent;

	boost::thread m_threadClient;
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

		LOG( DEBUG, "//////////JOB #"<<k<<"////////////");

		std::string job_status = ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		nTrials = 0;
		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Canceled") == std::string::npos)
		{
			try {
				job_status = ptrCli->queryJob(job_id_user);
				LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

				boost::this_thread::sleep(boost::posix_time::seconds(1));
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
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			//if(nTrials++ > NMAXTRIALS)
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
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			//if(nTrials++ > NMAXTRIALS)
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
	// kernel->load_plugin (TESTS_GUI_PLUGIN_PATH);
	kernel->load_plugin (TESTS_DRTS_PLUGIN_PATH);
	kernel->load_plugin (TESTS_FVM_FAKE_PLUGIN_PATH);

	return kernel;
}


int MyFixture::subscribe_and_wait ( const std::string &job_id, const sdpa::client::ClientApi::ptr_t &ptrCli )
{
	typedef boost::posix_time::ptime time_type;
	time_type poll_start = boost::posix_time::microsec_clock::local_time();

	int exit_code(4);

	bool bSubscribed = false;

	do
	{
		try
		{
			ptrCli->subscribe(job_id);
			bSubscribed = true;
		}
		catch(...)
		{
			bSubscribed = false;
			boost::this_thread::sleep(boost::posix_time::seconds(1));
		}

	}while(!bSubscribed);

	if(bSubscribed)
		LOG(INFO, "The client successfully subscribed for orchestrator notifications ...");

	std::string job_status;

  	int nTrials = 0;
  	do {

  		LOG(INFO, "start waiting at: " << poll_start);

  		try
  		{
  			if(nTrials<NMAXTRIALS)
			{
				boost::this_thread::sleep(boost::posix_time::seconds(3));
				LOG(INFO, "Re-trying ...");

				bSubscribed = false;

				do
				{
					try
					{
						ptrCli->subscribe(job_id);
						bSubscribed = true;
					}
					catch(...)
					{
						bSubscribed = false;
					}

				}while(!bSubscribed);

				if(bSubscribed)
					LOG(INFO, "The client successfully subscribed for orchestrator notifications ...");

			}

			seda::IEvent::Ptr reply( ptrCli->waitForNotification(0) );

			// check event type
			if (dynamic_cast<sdpa::events::JobFinishedEvent*>(reply.get()))
			{
				job_status="Finished";
				exit_code = 0;
			}
			else if (dynamic_cast<sdpa::events::JobFailedEvent*>(reply.get()))
			{
				job_status="Failed";
				exit_code = 1;
			}
			else if (dynamic_cast<sdpa::events::CancelJobAckEvent*>(reply.get()))
			{
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

BOOST_FIXTURE_TEST_SUITE( test_agents, MyFixture )

BOOST_AUTO_TEST_CASE( testPathOneDrts )
{

	//              O
	// 			    |
	// 			    |
	// 		        A
	//              |
	//              |   : -> variable agents #
	//              A
	//              |
	//              |
	//              N
	//

	LOG( DEBUG, "testPathOneDrts");

	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 	= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	//LOG( DEBUG, "Create Orchestrator with an Dummy workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	//LOG( DEBUG, "Create the Agent ...");
	boost::shared_array<sdpa::daemon::Agent::ptr_t> arrAgents( new sdpa::daemon::Agent::ptr_t[NAGENTS] );

	std::string strMaster = "orchestrator_0";
	for(int k=0; k<NAGENTS; k++)
	{
		ostringstream oss;
		oss<<"agent_"<<k;

		arrAgents[k] = sdpa::daemon::AgentFactory<DummyWorkflowEngine>::create( oss.str(),
																			  	addrAgent,
																			  	sdpa::master_info_list_t(1, MasterInfo(strMaster)),
																			  	MAX_CAP );

		// the master is the previous created agent
		strMaster = oss.str();
	}

	for(int k=0; k<NAGENTS; k++)
		arrAgents[k]->start_agent(false);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( create_drts("drts_0", strMaster) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );


	drts_0->stop();
	drts_0_thread.join();

	for(int k=0; k<NAGENTS; k++)
		arrAgents[k]->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testPathOneDrts terminated!");
}

BOOST_AUTO_TEST_CASE( testMultipleMastersOneDrts )
{
  //     O
  //     |
  //     +
  //  ./ | \.
  //  A  A  A    -> variable agents #
  //   \ | /
  //    \|/
  //     N

	LOG( DEBUG, "testMultipleMastersOneDrts");

	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent 		= "127.0.0.1";
	string addrNRE 		= "127.0.0.1";

	typedef void OrchWorkflowEngine;

	m_strWorkflow = read_workflow("workflows/stresstest.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	//LOG( DEBUG, "Create Orchestrator with an Dummy workflow engine ...");
	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false);

	//LOG( DEBUG, "Create the Agent ...");
	//sdpa::daemon::Agent::ptr_t arrAgents[NAGENTS];
	boost::shared_array<sdpa::daemon::Agent::ptr_t> arrAgents( new sdpa::daemon::Agent::ptr_t[NAGENTS] );

	std::string strMaster = "orchestrator_0";
	std::ostringstream ossDrtsMasterInfo;
	for(int k=0; k<NAGENTS; k++)
	{
		ostringstream oss;
		oss<<"agent_"<<k;

		ossDrtsMasterInfo<<oss.str();
		if(k!=NAGENTS-1)
			ossDrtsMasterInfo<<",";

		arrAgents[k] = sdpa::daemon::AgentFactory<DummyWorkflowEngine>::create( oss.str(),
																			  	addrAgent,
																			  	sdpa::master_info_list_t(1, MasterInfo(strMaster)),
																			  	MAX_CAP );

	}

	for(int k=0; k<NAGENTS; k++)
		arrAgents[k]->start_agent(false);

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( create_drts("drts_0", ossDrtsMasterInfo.str()) );
	boost::thread drts_0_thread = boost::thread(&fhg::core::kernel_t::run, drts_0);

	boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this));
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	threadClient.join();
	LOG( INFO, "The client thread joined the main thread°!" );

	drts_0->stop();
	drts_0_thread.join();

	for(int k=0; k<NAGENTS; k++)
		arrAgents[k]->shutdown();
	ptrOrch->shutdown();

	LOG( DEBUG, "The test case testMultipleMastersOneDrts terminated!");
}

BOOST_AUTO_TEST_SUITE_END()
