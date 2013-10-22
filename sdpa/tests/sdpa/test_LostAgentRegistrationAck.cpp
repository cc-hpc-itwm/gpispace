#define BOOST_TEST_MODULE test_LostRegistrationAck
#include <sdpa/daemon/JobFSM.hpp>
#include <boost/test/unit_test.hpp>

#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>
#include "tests_config.hpp"
#include <boost/filesystem/fstream.hpp>
#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include "kvs_setup_fixture.hpp"

namespace bfs=boost::filesystem;
using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

const int NMAXTRIALS = 5;
const int MAX_CAP 	 = 100;
static int testNb 	 = 0;

namespace po = boost::program_options;

#define NO_GUI ""

BOOST_GLOBAL_FIXTURE (KVSSetup);

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000) //microseconds
			, m_arrAggMasterInfo(1, MasterInfo("orchestrator_0"))
	{

		LOG(DEBUG, "Fixture's constructor called ...");
	}

	~MyFixture()
	{
		LOG(DEBUG, "Fixture's destructor called ...");

		seda::StageRegistry::instance().stopAll();
		seda::StageRegistry::instance().clear();
	}

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

	sdpa::master_info_list_t m_arrAggMasterInfo;

	std::string strBackupOrch;
	std::string strBackupAgent;

	boost::thread m_threadClient;
};


/*returns: 0 job finished, 1 job failed, 2 job cancelled, other value if failures occurred */
int subscribe_and_wait ( const std::string &job_id, const sdpa::client::ClientApi::ptr_t &ptrCli,  bool& bForceExit  )
{
	typedef boost::posix_time::ptime time_type;
	time_type poll_start = boost::posix_time::microsec_clock::local_time();

	int exit_code(4);
	std::string job_status;
	bool bSubscribed=false;

  	int nTrials = 0;
  	do
  	{
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

			if(bSubscribed)
				break;

			nTrials++;
			boost::this_thread::sleep(boost::posix_time::seconds(1));

		}while(nTrials<NMAXTRIALS);

		if(bSubscribed)
		{
			LOG(INFO, "The client successfully subscribed for orchestrator notifications ...");
			nTrials = 0;
		}
		else
		{
			LOG(INFO, "Could not connect to the orchestrator. Giving-up, now!");
		  	return exit_code;
		}


  		LOG(INFO, "start waiting at: " << poll_start);

  		try
  		{
  			if(nTrials<NMAXTRIALS)
  			{
  				boost::this_thread::sleep(boost::posix_time::seconds(1));
  				LOG(INFO, "Re-trying ...");
  			}

			seda::IEvent::Ptr reply( ptrCli->waitForNotification(1000000) );

			// check event type
			if (dynamic_cast<sdpa::events::JobFinishedEvent*>(reply.get()))
			{
				job_status="Finished";
				LOG(WARN, "The job has finished!");
				exit_code = 0;
			}
			else if (dynamic_cast<sdpa::events::JobFailedEvent*>(reply.get()))
			{
				job_status="Failed";
				LOG(WARN, "The job has failed!");
				exit_code = 1;
			}
			else if (dynamic_cast<sdpa::events::CancelJobAckEvent*>(reply.get()))
			{
				LOG(WARN, "The job has been canceled!");
				job_status="Cancelled";
				exit_code = 2;
			}
			else if(sdpa::events::ErrorEvent *err = dynamic_cast<sdpa::events::ErrorEvent*>(reply.get()))
			{
				LOG(WARN, "got error event: reason := "
							+ err->reason()
							+ " code := "
							+ boost::lexical_cast<std::string>(err->error_code()));

				// give some time to the orchestrator to come up
				boost::this_thread::sleep(boost::posix_time::seconds(3));

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

  	}while(exit_code == 4 && ++nTrials<NMAXTRIALS && !bForceExit );

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


const int NMAXFAIL=5;

template <typename T> class FaultyAgentFactory;

class FaultyAgent : public sdpa::daemon::Agent
{
public:
	typedef sdpa::shared_ptr<FaultyAgent > ptr_t;

	FaultyAgent(const std::string& name = "",
		  const std::string& url = "",
		  const sdpa::master_info_list_t arrMasterNames = sdpa::master_info_list_t(),
		  unsigned int cap = 10000,
		  bool bCanRunTasksLocally = false,
		  std::string strWorkflow = "")
	: Agent(name, url, arrMasterNames, cap, bCanRunTasksLocally)
	, nSuccFailures_(0)
	, strWorkflow_(strWorkflow)
	, bForceExit_(false)
	{
		threadClient = boost::thread(boost::bind(&FaultyAgent::start_client, this));
	}

	void start_client()
	{
		sdpa::client::config_t config = sdpa::client::ClientApi::config();

		std::vector<std::string> cav;
		cav.push_back("--orchestrator=orchestrator_0");
		cav.push_back("--network.timeout=-1");
		config.parse_command_line(cav);

		std::ostringstream osstr;
		osstr<<"sdpac_"<<testNb++;

		sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config, osstr.str(), osstr.str()+".apps.client.out" );
		ptrCli->configure_network( config );

		int nTrials = 0;
		sdpa::job_id_t job_id_user;

		try {

			LOG( DEBUG, "Submitting new workflow ...");
			job_id_user = ptrCli->submitJob(strWorkflow_);
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

		subscribe_and_wait( job_id_user, ptrCli, bForceExit_ );
	}

	void stop_client()
	{
		LOG( DEBUG, "Stop the client now ...");
		bForceExit_ = true;
		threadClient.interrupt();
		if(threadClient.joinable())
			threadClient.join();
	}

	void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent* pRegAckEvt)
	{
		std::string masterName = pRegAckEvt->from();
		LOG(INFO, "Received registration acknowledgment from "<<masterName<<" ... and ignore it!");

		if( nSuccFailures_<NMAXFAIL )
			nSuccFailures_++;
		else
		{
			//shutdown();
			stop_client();
		}
	}

	template <typename T> friend class FaultyAgentFactory;

  private:
	int nSuccFailures_;
	std::string strWorkflow_;
	boost::thread threadClient;
	bool bForceExit_;
};

template <typename T>
class FaultyAgentFactory
{
public:
   static FaultyAgent::ptr_t create( const std::string& name,
							   const std::string& url,
							   const sdpa::master_info_list_t& arrMasterNames,
							   const unsigned int capacity )
   {
	   LOG( DEBUG, "Create agent \""<<name<<"\" with an workflow engine of type "<<typeid(T).name() );
	   FaultyAgent::ptr_t pAgent( new FaultyAgent( name, url, arrMasterNames, capacity ) );
	   pAgent->createWorkflowEngine<T>();

	   seda::IEventQueue::Ptr ptrEvtQueue(new seda::EventQueue("network.stage."+name+".queue", agent::MAX_Q_SIZE));
	   seda::Stage::Ptr daemon_stage( new seda::Stage(name, ptrEvtQueue, pAgent, 1) );

	   pAgent->setStage(daemon_stage);
	   seda::StageRegistry::instance().insert(daemon_stage);

	   return pAgent;
   }
};

BOOST_FIXTURE_TEST_SUITE( test_LostRegistrationAck, MyFixture );

BOOST_AUTO_TEST_CASE( testLostRegAck)
{
	LOG( DEBUG, "testLostRegAck");

	//guiUrl
	string guiUrl   	= "";
	string workerUrl 	= "127.0.0.1:5500";
	string addrOrch 	= "127.0.0.1";
	string addrAgent0 	= "127.0.0.1";
	string addrAgent1	= "127.0.0.1";

	std::string strBackupAgent0;
	std::string strBackupAgent1;

	m_strWorkflow = read_workflow("workflows/transform_file.pnet");
	LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

	sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create("orchestrator_0", addrOrch, MAX_CAP);
	ptrOrch->start_agent(false, strBackupOrch);

	sdpa::master_info_list_t arrAgent0MasterInfo(1, MasterInfo("orchestrator_0"));
	sdpa::daemon::Agent::ptr_t ptrAgent0 = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_0", addrAgent0, arrAgent0MasterInfo, MAX_CAP );
	ptrAgent0->start_agent(false, strBackupAgent0);

	// create faulty agent
	sdpa::master_info_list_t arrAgent1MasterInfo(1, MasterInfo("agent_0"));
	FaultyAgent::ptr_t ptrAgent1 = FaultyAgentFactory<EmptyWorkflowEngine>::create("agent_1", addrAgent1, arrAgent1MasterInfo, MAX_CAP );

	ptrAgent1->start_agent(false, strBackupAgent1);

	ptrOrch->shutdown();
	ptrAgent0->shutdown();
	ptrAgent1->shutdown();
	LOG( DEBUG, "The test case testLostRegAck terminated!");
}

BOOST_AUTO_TEST_SUITE_END()
