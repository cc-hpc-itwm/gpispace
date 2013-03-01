/*
 * =====================================================================================
 *
 *       Filename:  test_MapReduce.cpp
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
#define BOOST_TEST_MODULE testMapReduce
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
#include "sdpa/daemon/DaemonFSM.hpp"
#include <seda/Strategy.hpp>
#include <sdpa/client/ClientApi.hpp>

#include <plugins/drts.hpp>
#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <seda/StageRegistry.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>

#include <sdpa/engine/MasterWorkflowEngine.hpp>
#include <sdpa/engine/CollectorWorkflowEngine.hpp>
#include <sdpa/engine/MapperWorkflowEngine.hpp>
#include <sdpa/engine/ReducerWorkflowEngine.hpp>

#include <sdpa/mapreduce/WordCountMapper.hpp>
#include <sdpa/mapreduce/WordCountReducer.hpp>

#include <boost/thread.hpp>

//plugin
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>


const int NMAXTRIALS=5;
const int MAX_CAP = 100;
static int testNb = 0;

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
			, m_arrAggMasterInfo(1, MasterInfo("orchestrator_0"))
	{ //initialize and start_agent the finite state machine

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

	}

	~MyFixture()
	{
		LOG(DEBUG, "Fixture's destructor called ...");

		sstrOrch.str("");
		sstrAgg.str("");

		m_serv->stop ();
		m_pool->stop ();
		if(m_thrd->joinable())
			m_thrd->join ();

		delete m_thrd;
		delete m_serv;
		delete m_kvsd;
		delete m_pool;

		seda::StageRegistry::instance().clear();
	}

	void run_client(const std::string& strWorkflow );

  void run_client_subscriber(const std::string& strWorkflow );
  int subscribe_and_wait( const std::string&, const sdpa::client::ClientApi::ptr_t& );

	int m_nITER;
	int m_sleep_interval ;

  fhg::com::io_service_pool *m_pool;
	fhg::com::kvs::server::kvsd *m_kvsd;
	fhg::com::tcp_server *m_serv;
	boost::thread *m_thrd;

	sdpa::master_info_list_t m_arrAggMasterInfo;

	std::stringstream sstrOrch;
	std::stringstream sstrAgg;

	boost::thread m_threadClient;

	fhg::core::kernel_t *kernel;
};

string read_wordcount_file(const string& strFileName)
{
  MapTask<std::string, std::string, std::string, std::string> mapTask;
  ifstream ifs;
  ofstream ofs;
  std::string strWorkflow("");

  ifs.open(strFileName.c_str(), ifstream::in);
  if(!ifs.fail())
  {
    string strLine;

    while( getline( ifs, strLine ) )
    {
      boost::char_separator<char> sep(" ");
      boost::tokenizer<boost::char_separator<char> > tok(strLine, sep);

      vector< string > vec;
      vec.assign(tok.begin(),tok.end());

      if( vec.size() != 2 )
      {
        LOG(ERROR, "Invalid line in file "<<strFileName<<". Please specify on each line a filename followed by the node name");
        return "";
      }
      else
      {
        mapTask.emit(vec[0], vec[1]);
      }
    }

    mapTask.print();
    strWorkflow = mapTask.encode();
  }
  else
  {
    cout<<"Error, the file "<<strFileName<<" does not exist!"<<endl;
  }

  ifs.close();

  return strWorkflow;
}

void MyFixture::run_client(const std::string& strWorkflow )
{
	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	config.parse_command_line(cav);

	std::ostringstream osstr;
	osstr<<"sdpac_"<<testNb++;

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config, osstr.str(), osstr.str()+".apps.client.out" );
	ptrCli->configure_network( config );

	for( int k=0; k<m_nITER; k++ )
	{
		int nTrials = 0;
		sdpa::job_id_t job_id_user;

		try {

			LOG( DEBUG, "Submitting the following test workflow: \n"<<strWorkflow);

			job_id_user = ptrCli->submitJob(strWorkflow);
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
			   job_status.find("Cancelled") == std::string::npos)
		{
			try {
				job_status = ptrCli->queryJob(job_id_user);
				LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

				boost::this_thread::sleep(boost::posix_time::seconds(10));
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
				boost::this_thread::sleep(boost::posix_time::seconds(3));
		}
		catch(const sdpa::client::ClientException& cliExc)
		{

			LOG( DEBUG, "The maximum number of trials was exceeded. Giving-up now!");

			ptrCli->shutdown_network();
			ptrCli.reset();
			return;

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
			LOG( DEBUG, "The maximum number of  trials was exceeded. Giving-up now!");

			ptrCli->shutdown_network();
			ptrCli.reset();
			return;

			boost::this_thread::sleep(boost::posix_time::seconds(3));
		}
	}

	ptrCli->shutdown_network();
	boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
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
    do
    {

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
        if (sdpa::events::JobFinishedEvent* pJobFinished = dynamic_cast<sdpa::events::JobFinishedEvent*>(reply.get()))
        {
          LOG(INFO, "The result of the mapreduce job is stored within the file "<<pJobFinished->result());
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

void MyFixture::run_client_subscriber(const std::string& strWorkflow )
{
  sdpa::client::config_t config = sdpa::client::ClientApi::config();

  std::vector<std::string> cav;
  cav.push_back("--orchestrator=orchestrator_0");
  cav.push_back("--network.timeout=0");
  config.parse_command_line(cav);

  std::ostringstream osstr;
  osstr<<"sdpac_"<<testNb++;

  sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config, osstr.str(), osstr.str()+".apps.client.out" );
  ptrCli->configure_network( config );

  for( int k=0; k<m_nITER; k++ )
  {
    int nTrials = 0;
    sdpa::job_id_t job_id_user;

    try {

      LOG( DEBUG, "Submitting new workflow ..."); //<<m_strWorkflow);
      job_id_user = ptrCli->submitJob(strWorkflow);
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

    int exit_code = subscribe_and_wait( job_id_user, ptrCli );

    try {
      LOG( DEBUG, "User: delete the job "<<job_id_user);
      ptrCli->deleteJob(job_id_user);
      //boost::this_thread::sleep(boost::posix_time::seconds(3));
    }
    catch(const sdpa::client::ClientException& cliExc)
    {
      LOG( DEBUG, "The maximum number of  trials was exceeded. Giving-up now!");

      ptrCli->shutdown_network();
      ptrCli.reset();
      return;

      boost::this_thread::sleep(boost::posix_time::seconds(3));
    }
  }

  ptrCli->shutdown_network();
  //boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
  ptrCli.reset();
}

BOOST_FIXTURE_TEST_SUITE( test_agents, MyFixture )

BOOST_AUTO_TEST_CASE( testMapTaskEncoding )
{
  LOG( DEBUG, "***** testMapReduce *****"<<std::endl);
  //guiUrl
  string guiUrl     = "";
  string workerUrl  = "127.0.0.1:5500";
  string addrOrch   = "127.0.0.1";
  string addrAgent  = "127.0.0.1";

  boost::filesystem::path file("mapreduce.out");
  if(boost::filesystem::exists(file))
    boost::filesystem::remove(file);

  // one should have a Partitioner
  // cases to distinguish: 1) files are stored locally, on some given nodes
  //                       2) are stored on a (distributed) file system

  WordCountMapper::TaskT mapTask1("file1.txt", "node1");

  LOG(INFO, "Before serialization, the map task looks as follows: ");
  mapTask1.print();

  std::string strWorkflow = mapTask1.encode();

  WordCountMapper::TaskT mapTask2;
  mapTask2.decode(strWorkflow);

  LOG(INFO, "After de-serialization, the map task looks as follows: ");
  mapTask2.print();
}

BOOST_AUTO_TEST_CASE( testMapperEncoding )
{
  LOG( DEBUG, "***** testMapReduce *****"<<std::endl);
  //guiUrl
  string guiUrl     = "";
  string workerUrl  = "127.0.0.1:5500";
  string addrOrch   = "127.0.0.1";
  string addrAgent  = "127.0.0.1";

  boost::filesystem::path file("reducer.out");
  if(boost::filesystem::exists(file))
    boost::filesystem::remove(file);

  // one should have a Partitioner
  // cases to distinguish: 1) files are stored locally, on some given nodes
  //                       2) are stored on a (distributed) file system

  WordCountMapper::TaskT mapTask1("file1.txt", "node1");
  WordCountMapper::TaskT mapTask2("file2.txt", "node2");

  WordCountMapper wcMapper;
  wcMapper.addTask(mapTask1.inKey(), mapTask1);
  wcMapper.addTask(mapTask2.inKey(), mapTask2);

  std::string strWorkflow = wcMapper.encode();

  WordCountMapper mapper;
  mapper.decode(strWorkflow);

  LOG(INFO, "After de-serialization, the mapper looks as follows: ");
  mapper.print();
}

BOOST_AUTO_TEST_CASE( test2Mappers3Reducers )
{
  LOG( DEBUG, "***** testWordCount *****");
  //guiUrl
  string guiUrl     = "";
  string workerUrl  = "127.0.0.1:5500";
  string addrOrch   = "127.0.0.1";
  string addrAgent  = "127.0.0.1";

  std::string strWorkflow = read_wordcount_file("wordcount_test.txt");

  sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<MasterWorkflowEngine<WordCountMapper> >::create("orchestrator_0", addrAgent, MAX_CAP );
  ptrOrch->start_agent(false);

  sdpa::master_info_list_t arrInfoOrch(1, MasterInfo("orchestrator_0"));
  sdpa::daemon::Agent::ptr_t ptrMapper0 = sdpa::daemon::AgentFactory<MapperWorkflowEngine<WordCountMapper> >::create("mapper0", addrAgent, arrInfoOrch, MAX_CAP);
  ptrMapper0->addCapability(sdpa::capability_t("mapper", "node", "mapper0"));
  ptrMapper0->start_agent(false);

  sdpa::daemon::Agent::ptr_t ptrMapper1 = sdpa::daemon::AgentFactory<MapperWorkflowEngine<WordCountMapper> >::create("mapper1", addrAgent, arrInfoOrch, MAX_CAP);
  ptrMapper1->addCapability( sdpa::capability_t("mapper", "node", "mapper1"));
  ptrMapper1->start_agent(false);

  sdpa::master_info_list_t arrInfoMappers;
  MasterInfo m1("mapper0");
  MasterInfo m2("mapper1");
  arrInfoMappers.push_back(m1);
  arrInfoMappers.push_back(m2);

  sdpa::daemon::Agent::ptr_t ptrReducer0 = sdpa::daemon::AgentFactory<ReducerWorkflowEngine<WordCountMapper, WordCountReducer> >::create("reducer0", addrAgent, arrInfoMappers, MAX_CAP );
  ptrReducer0->addCapability( sdpa::capability_t("reducer0", "node", "reducer0"));
  ptrReducer0->start_agent(false);

  sdpa::daemon::Agent::ptr_t ptrReducer1 = sdpa::daemon::AgentFactory<ReducerWorkflowEngine<WordCountMapper, WordCountReducer> >::create("reducer1", addrAgent, arrInfoMappers, MAX_CAP);
  ptrReducer1->addCapability(sdpa::capability_t("reducer1", "node", "reducer1"));
  ptrReducer1->start_agent(false);

  sdpa::daemon::Agent::ptr_t ptrReducer2 = sdpa::daemon::AgentFactory<ReducerWorkflowEngine<WordCountMapper, WordCountReducer> >::create("reducer2", addrAgent, arrInfoMappers, MAX_CAP);
  ptrReducer2->addCapability(sdpa::capability_t("reducer2", "node", "reducer2"));
  ptrReducer2->start_agent(false);

  sdpa::master_info_list_t arrInfoReducers;
  MasterInfo m5("reducer0");
  MasterInfo m6("reducer1");
  MasterInfo m7("reducer2");
  arrInfoReducers.push_back(m5);
  arrInfoReducers.push_back(m6);
  arrInfoReducers.push_back(m7);

  sdpa::daemon::Agent::ptr_t ptrCollector = sdpa::daemon::AgentFactory<CollectorWorkflowEngine<WordCountMapper, WordCountReducer> >::create("collector", addrAgent, arrInfoReducers, MAX_CAP );
  ptrCollector->start_agent(false);

  boost::filesystem::path file("reducer.out");
  if(boost::filesystem::exists(file))
    boost::filesystem::remove(file);

  boost::this_thread::sleep(boost::posix_time::seconds(3));
  boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client_subscriber, this, strWorkflow));
  threadClient.join();
  LOG( INFO, "The client thread joined the main thread!" );

  boost::this_thread::sleep(boost::posix_time::seconds(1));
  ptrCollector->shutdown(); // first should wait to finish!!!
  ptrReducer2->shutdown();
  ptrReducer1->shutdown();
  ptrReducer0->shutdown();
  ptrMapper1->shutdown();
  ptrMapper0->shutdown();
  ptrOrch->shutdown();
  boost::this_thread::sleep(boost::posix_time::seconds(1));

  LOG( DEBUG, "The test case testWordCount terminated!");
}

BOOST_AUTO_TEST_SUITE_END()
