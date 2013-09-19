///
 // =====================================================================================
 //
 //       Filename:  test_Topologies.cpp
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
#include <boost/test/unit_test.hpp>
#include "tests_config.hpp"
#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>
#include <sdpa/engine/DummyWorkflowEngine.hpp>
#include <tests/sdpa/CreateDrtsWorker.hpp>
#include "kvs_setup_fixture.hpp"
#include <boost/shared_array.hpp>

const int NMAXTRIALS=5;
const int MAX_CAP = 100;
const int NAGENTS = 1;

const int BUNCH_SIZE = 1;

namespace po = boost::program_options;

using namespace std;

#define NO_GUI ""

BOOST_GLOBAL_FIXTURE (KVSSetup);

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
			, m_arrAgentMasterInfo(1, MasterInfo("orchestrator_0"))
	{
		LOG(DEBUG, "Fixture's constructor called ...");
		m_strWorkflow = read_workflow("workflows/transform_file.pnet");
	}

	~MyFixture()
	{
		LOG(DEBUG, "Fixture's destructor called ...");

		sstrOrch.str("");
		sstrAgent.str("");

		seda::StageRegistry::instance().stopAll();
		seda::StageRegistry::instance().clear();
	}

	void run_client();
	void run_cannon_client();

	int subscribe_and_wait ( const std::string &job_id, const sdpa::client::ClientApi::ptr_t &ptrCli );

	string read_workflow(string strFileName)
	{
		ifstream f(strFileName.c_str());
		ostringstream os;
		os.str("");

		BOOST_REQUIRE (f.is_open());

    char c;
    while (f.get(c)) os<<c;
    f.close();

		return os.str();
	}

	int m_nITER;
	int m_sleep_interval ;
    std::string m_strWorkflow;

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

				boost::this_thread::sleep(boost::posix_time::seconds(1));
			}
		}

		nTrials = 0;

		try {
				LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
				ptrCli->retrieveResults(job_id_user);
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

			boost::this_thread::sleep(boost::posix_time::seconds(1));
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

			boost::this_thread::sleep(boost::posix_time::seconds(1));
		}
	}

	ptrCli->shutdown_network();
	ptrCli.reset();
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

			seda::IEvent::Ptr reply( ptrCli->waitForNotification(10000) );

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

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");
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

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", strMaster, "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
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

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");
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

	sdpa::shared_ptr<fhg::core::kernel_t> drts_0( createDRTSWorker("drts_0", ossDrtsMasterInfo.str(), "", TESTS_TRANSFORM_FILE_MODULES_PATH, kvs_host(), kvs_port()) );
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
