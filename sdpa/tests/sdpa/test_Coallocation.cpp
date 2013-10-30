/*
 * =====================================================================================
 *
 *       Filename:  test_Coallocation.cpp
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
#define BOOST_TEST_MODULE testCoallocation
#include <sdpa/daemon/JobFSM.hpp>
#include <boost/test/unit_test.hpp>
#include "tests_config.hpp"
#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>
#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>
#include <tests/sdpa/CreateDrtsWorker.hpp>
#include "kvs_setup_fixture.hpp"

const int NMAXTRIALS=5;
const int MAX_CAP = 100;
static int testNb = 0;

namespace po = boost::program_options;

using namespace std;
using namespace sdpa::daemon;

#define NO_GUI ""

BOOST_GLOBAL_FIXTURE (KVSSetup);

struct MyFixture
{
  MyFixture()
            :m_sleep_interval(1000) //microseconds
  {
    LOG(DEBUG, "Fixture's constructor called ...");
  }

  ~MyFixture()
  {
    LOG(DEBUG, "Fixture's destructor called ...");

    seda::StageRegistry::instance().stopAll();
    seda::StageRegistry::instance().clear();
    testNb++;
  }

  void run_client(const std::string&, const std::string&);
  int subscribe_and_wait ( const std::string &job_id, const sdpa::client::ClientApi::ptr_t &ptrCli );

  string read_workflow(string strFileName)
  {
    ifstream f(strFileName.c_str());
    ostringstream os;
    os.str("");

    char c;
    while (f.get(c))
      os<<c;

    f.close();
    return os.str();
  }

  int m_sleep_interval ;
  std::string m_strWorkflow;

  std::string strBackupOrch;
  std::string strBackupAgent;

  boost::thread m_threadClient;
};

/*returns: 0 job finished, 1 job failed, 2 job cancelled, other value if failures occurred */
int MyFixture::subscribe_and_wait ( const std::string &job_id, const sdpa::client::ClientApi::ptr_t &ptrCli )
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

void MyFixture::run_client(const std::string& orchName, const std::string& cliName)
{
  sdpa::client::config_t config = sdpa::client::ClientApi::config();

  std::vector<std::string> cav;
  std::string prefix("--orchestrator=");
  cav.push_back(prefix+orchName);
  //cav.push_back("--network.timeout=-1");
  config.parse_command_line(cav);

  sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config, cliName, cliName+".apps.client.out" );
  ptrCli->configure_network( config );

  int nTrials = 0;
  sdpa::job_id_t job_id_user;

  try {

    LOG( INFO, "Submitting new workflow ..."); //<<m_strWorkflow);
    job_id_user = ptrCli->submitJob(m_strWorkflow);
  }
  catch(const sdpa::client::ClientException& cliExc)
  {
    if(nTrials++ > NMAXTRIALS) {
      LOG( WARN, "The maximum number of job submission  trials was exceeded. Giving-up now!");

      ptrCli->shutdown_network();
      ptrCli.reset();
      return;
    }

    boost::this_thread::sleep(boost::posix_time::seconds(1));
  }

  subscribe_and_wait( job_id_user, ptrCli );

  try {
    LOG( INFO, "The client requests to delete the job "<<job_id_user);
    ptrCli->deleteJob(job_id_user);
  }
  catch(const sdpa::client::ClientException& cliExc)
  {
    LOG( WARN, "The maximum number of  trials was exceeded. Giving-up now!");

    ptrCli->shutdown_network();
    ptrCli.reset();
    boost::this_thread::sleep(boost::posix_time::seconds(1));
    return;
  }

  ptrCli->shutdown_network();
  ptrCli.reset();
}


BOOST_FIXTURE_TEST_SUITE( test_coallocation, MyFixture )

BOOST_AUTO_TEST_CASE(testCollocSched)
{
  LOG(INFO, "Test the co-allocation ...");

  const int NWORKERS = 12;
  const std::string WORKER_CPBS[] = {"A", "B", "C"};

  string addrAg = "127.0.0.1";
  string strBackupOrch;
  ostringstream oss;

  sdpa::master_info_list_t arrAgentMasterInfo;
  sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

  pAgent-> createScheduler(false);

  if(!pAgent->scheduler())
  LOG(FATAL, "The scheduler was not properly initialized");

  // add a couple of workers
  for( int k=0; k<NWORKERS; k++ )
  {
    oss.str("");
    oss<<k;

    sdpa::worker_id_t workerId(oss.str());
    std::string cpbName(WORKER_CPBS[k%3]);
    sdpa::capability_t cpb(cpbName, "virtual", workerId);
    sdpa::capabilities_set_t cpbSet;
    cpbSet.insert(cpb);
    pAgent->scheduler()->addWorker(workerId, 1, cpbSet);
  }

  // create a number of jobs
  const sdpa::job_id_t jobId0("Job0");
  sdpa::daemon::Job::ptr_t pJob0(new JobFSM(jobId0, "description 0"));
  pAgent->jobManager()->addJob(jobId0, pJob0);

  const sdpa::job_id_t jobId1("Job1");
  sdpa::daemon::Job::ptr_t pJob1(new JobFSM(jobId1, "description 1"));
  pAgent->jobManager()->addJob(jobId1, pJob1);

  const sdpa::job_id_t jobId2("Job2");
  sdpa::daemon::Job::ptr_t pJob2(new JobFSM(jobId2, "description 2"));
  pAgent->jobManager()->addJob(jobId2, pJob2);

  job_requirements_t jobReqs_0(requirement_list_t(1, requirement_t(WORKER_CPBS[0], true)), schedule_data(4, 100));
  pAgent->jobManager()->addJobRequirements(jobId0, jobReqs_0);
  pAgent->scheduler()->schedule_remotely(jobId0);

  job_requirements_t jobReqs_1(requirement_list_t(1, requirement_t(WORKER_CPBS[1], true)), schedule_data(4, 100));
  pAgent->jobManager()->addJobRequirements(jobId1, jobReqs_1);
  pAgent->scheduler()->schedule_remotely(jobId1);

  job_requirements_t jobReqs_2(requirement_list_t(1, requirement_t(WORKER_CPBS[2], true)), schedule_data(4, 100));
  pAgent->jobManager()->addJobRequirements(jobId2, jobReqs_2);
  pAgent->scheduler()->schedule_remotely(jobId2);

  pAgent->scheduler()->assignJobsToWorkers();

  ostringstream ossrw;int k=-1;
  ossrw<<std::setfill (' ')<<std::setw(2);
  sdpa::worker_id_list_t listJobAssignedWorkers = pAgent->scheduler()->getListAllocatedWorkers(jobId0);
  BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
  {
    k = boost::lexical_cast<int>(wid);
    BOOST_CHECK( k==0 || k==3 || k==6 || k==9);
    ossrw<<wid<<" ";
  }
  //LOG(INFO, "The job jobId0 has been allocated the workers "<<ossrw.str());

  ossrw.str(""); listJobAssignedWorkers.clear();
  listJobAssignedWorkers = pAgent->scheduler()->getListAllocatedWorkers(jobId1);
  BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
  {
    k = boost::lexical_cast<int>(wid);
    BOOST_CHECK( k==1 || k==4 || k==7 || k==10);
    ossrw<<std::setfill (' ')<<std::setw(10)<<wid<<" ";
  }
  //LOG(INFO, "The job jobId1 has been allocated the workers "<<ossrw.str());

  ossrw.str(""); listJobAssignedWorkers.clear();
  listJobAssignedWorkers = pAgent->scheduler()->getListAllocatedWorkers(jobId2);
  BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
  {
    k = boost::lexical_cast<int>(wid);
    BOOST_CHECK( k==2 || k==5 || k==8 || k==11);
    ossrw<<wid<<" ";
  }
  //LOG(INFO, "The job jobId2 has been allocated the workers "<<ossrw.str());

  // try now to schedule a job requiring 2 resources of type "A"
  const sdpa::job_id_t jobId4("Job4");
  sdpa::daemon::Job::ptr_t pJob4(new JobFSM(jobId4, "description 4"));
  pAgent->jobManager()->addJob(jobId4, pJob4);

  job_requirements_t jobReqs_5(requirement_list_t(1, requirement_t(WORKER_CPBS[0], true)), schedule_data(2, 100));
  pAgent->jobManager()->addJobRequirements(jobId4, jobReqs_5);
  pAgent->scheduler()->schedule_remotely(jobId4);

  pAgent->scheduler()->assignJobsToWorkers();
  sdpa::worker_id_list_t listFreeWorkers(pAgent->scheduler()->getListAllocatedWorkers(jobId4));
  BOOST_CHECK(listFreeWorkers.empty());

  //reinterpret_cast<SchedulerImpl*>(pAgent->scheduler().get())->printAllocationTable();

  // Now report that jobId0 has finished and try to assign again resources to the job 4
  pAgent->scheduler()->releaseReservation(jobId0);

  //listFreeWorkers.clear();
  pAgent->scheduler()->assignJobsToWorkers();

  listFreeWorkers = pAgent->scheduler()->getListAllocatedWorkers(jobId4);
  BOOST_CHECK(!listFreeWorkers.empty());

  int w0 = boost::lexical_cast<int>(listFreeWorkers.front());
  BOOST_CHECK(w0==0 || w0 == 3  || w0 == 6|| w0 == 9);

  /*
  int w1 = boost::lexical_cast<int>(*(boost::next(listFreeWorkers.begin())));
  BOOST_CHECK(w1==0 || w1 == 3  || w1 == 6|| w1 == 9);
  */
}

BOOST_AUTO_TEST_CASE( testCoallocationWorkflow )
{
  LOG( INFO, "***** Test capabilities *****"<<std::endl);

  const int NWORKERS=5;

  //guiUrl
  string guiUrl   	= "";
  string workerUrl 	= "127.0.0.1:5500";
  string addrOrch 	= "127.0.0.1";
  string addrAgent 	= "127.0.0.1";

  typedef void OrchWorkflowEngine;

  m_strWorkflow = read_workflow("workflows/coallocation_test.pnet");

  ostringstream osstr;
  osstr<<"orchestrator_"<<testNb;
  std::string orchName(osstr.str());

  osstr.str("");
  osstr<<"client_"<<testNb;
  std::string cliName(osstr.str());

  osstr.str("");
  osstr<<"agent_"<<testNb;
  std::string agentName(osstr.str());

  sdpa::daemon::Orchestrator::ptr_t ptrOrch = sdpa::daemon::OrchestratorFactory<void>::create(orchName, addrOrch, MAX_CAP);
  ptrOrch->start_agent(false);

  sdpa::master_info_list_t arrAgentMasterInfo(1, sdpa::MasterInfo(orchName));
  sdpa::daemon::Agent::ptr_t ptrAgent = sdpa::daemon::AgentFactory<we::mgmt::layer>::create(agentName, addrAgent, arrAgentMasterInfo, MAX_CAP );
  ptrAgent->start_agent(false);

  boost::thread drts_thread[NWORKERS];
  sdpa::shared_ptr<fhg::core::kernel_t> drts[NWORKERS];

  ostringstream oss; int i;
  for(i=0;i<2;i++)
  {
    oss<<"drts_"<<testNb<<"_"<<i;
    drts[i] = createDRTSWorker(oss.str(), agentName, "A", TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH, kvs_host(), kvs_port());
    drts_thread[i] = boost::thread( &fhg::core::kernel_t::run, drts[i] );
    oss.str("");
  }

  for(i=2;i<NWORKERS;i++)
  {
    oss<<"drts_"<<testNb<<"_"<<i;
    drts[i] = createDRTSWorker(oss.str(), agentName, "B", TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH, kvs_host(), kvs_port());
    drts_thread[i] = boost::thread( &fhg::core::kernel_t::run, drts[i] );
    oss.str("");
  }

  boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_client, this, orchName, cliName));

  if(threadClient.joinable())
    threadClient.join();
  LOG( INFO, "The client thread joined the main thread!" );

  for(i=2;i<NWORKERS;i++)
  {
    drts[i]->stop();
    if(drts_thread[i].joinable())
      drts_thread[i].join();
  }

  ptrAgent->shutdown();
  ptrOrch->shutdown();

  LOG( INFO, "The test case Test1 terminated!");
}

BOOST_AUTO_TEST_SUITE_END()
