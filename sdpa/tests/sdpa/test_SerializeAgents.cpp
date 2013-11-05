#define BOOST_TEST_MODULE TestAgentsSerialization

#include <boost/test/unit_test.hpp>

#include <sdpa/daemon/JobFSM.hpp>
#include <iostream>

#include <sdpa/daemon/Worker.hpp>
#include <sdpa/JobId.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "tests_config.hpp"

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
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/client/ClientApi.hpp>

#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include <sdpa/engine/DummyWorkflowEngine.hpp>
#include <tests/sdpa/CreateDrtsWorker.hpp>

#include "kvs_setup_fixture.hpp"
BOOST_GLOBAL_FIXTURE (KVSSetup);

const int NWORKERS = 12;
const int NJOBS    = 4;
const int MAX_CAP  = 100;

const std::string WORKER_CPBS[] = {"A", "B", "C"};


using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

struct MyFixture
{
  MyFixture()
    : m_nITER(1)
    , m_sleep_interval(1000000)
  { //initialize and start the finite state machine

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
};

BOOST_FIXTURE_TEST_SUITE( test_SerializeAgents, MyFixture );

BOOST_AUTO_TEST_CASE(testDummyWorkflowEngineSerialization)
{
  std::cout<<std::endl<<"----------------Begin  testDummyWorkflowEngineSerialization----------------"<<std::endl;

  std::string filename = "testDummyWorkflowEngineSerialization.txt"; // = boost::archive::tmpdir());filename += "/testfile";

  we::mgmt::basic_layer* pWfEng = new DummyWorkflowEngine();

  const int NWFS = 10;
  encoded_type wf_desc("workflow description ");

  for(int i=0; i<NWFS; i++) {
      sdpa::job_id_t jobId;
      pWfEng->submit(jobId.str(), wf_desc, we::type::user_data ());
  }

  // serialize now the job queue
  try
  {
      std::cout<<"----------------The DummyWorkflowEngine's content before the backup is:----------------"<<std::endl;
      dynamic_cast<DummyWorkflowEngine*>(pWfEng)->print();
      std::ofstream ofs(filename.c_str());
      boost::archive::text_oarchive oa(ofs);
      oa.register_type(static_cast<DummyWorkflowEngine*>(NULL));
      oa<<pWfEng;
  }
  catch(exception &e)
  {
      cout <<"Exception occurred: "<< e.what() << endl;
      return;
  }

  // restore state to one equivalent to the original
  try
  {
      we::mgmt::basic_layer* pRestoredWfEng;
      // open the archive
      std::ifstream ifs(filename.c_str());
      boost::archive::text_iarchive ia(ifs);
      ia.register_type(static_cast<DummyWorkflowEngine*>(NULL));
      ia >> pRestoredWfEng;
      std::cout<<"----------------The DummyWorkflowEngine's content after the backup is:----------------"<<std::endl;
      dynamic_cast<DummyWorkflowEngine*>(pRestoredWfEng)->print();
  }
  catch(exception &e)
  {
      cout <<"Exception occurred: "<< e.what() << endl;
      return;
  }

  std::cout<<std::endl<<"----------------End  testDummyWorkflowEngineSerialization----------------"<<std::endl;
}

BOOST_AUTO_TEST_CASE(testAgentSerializationNoWfe)
{
  std::cout<<std::endl<<"----------------Begin  testAgentSerialization----------------"<<std::endl;
  std::string filename = "testSerializeAgent.txt"; // = boost::archive::tmpdir());filename += "/testfile";

  LOG(INFO, "Test the co-allocation ...");

  string addrAg = "127.0.0.1";
  string strBackupOrch;
  ostringstream oss;

  sdpa::master_info_list_t arrAgentMasterInfo;
  sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

  pAgent-> createScheduler(false);

  if(!pAgent->scheduler())
    LOG(FATAL, "The scheduler was not properly initialized");

  // add a couple of workers
  for( int k=0; k<NWORKERS; k++ ) {
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

  std::filebuf fb;
  fb.open (filename.c_str(),std::ios::out);
  std::ostream bkpos(&fb);
  pAgent->backup(bkpos);

  sdpa::daemon::Agent::ptr_t pNewAgent = sdpa::daemon::AgentFactory<void>::create("agent_007_rec", addrAg, arrAgentMasterInfo,  MAX_CAP);

  std::ifstream is(filename.c_str());
  pNewAgent->recover(is);
  is.close();
  std::cout<<"----------------The Agent's content after recovering is:----------------"<<std::endl;

  //pNewAgent->print();
  std::cout<<std::endl<<"----------------End  testAgentSerialization----------------"<<std::endl;
}

BOOST_AUTO_TEST_CASE(testAgentSerializationEmptyWfe)
{
  std::cout<<std::endl<<"----------------Begin  testAgentSerialization----------------"<<std::endl;
  std::string filename = "testSerializeAgent.txt"; // = boost::archive::tmpdir());filename += "/testfile";

  LOG(INFO, "Test the co-allocation ...");

  string addrAg = "127.0.0.1";
  string strBackupOrch;
  ostringstream oss;

  sdpa::master_info_list_t arrAgentMasterInfo;
  sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

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

  std::filebuf fb;
  fb.open (filename.c_str(),std::ios::out);
  std::ostream bkpos(&fb);
  pAgent->backup(bkpos);

  sdpa::daemon::Agent::ptr_t pNewAgent = sdpa::daemon::AgentFactory<EmptyWorkflowEngine>::create("agent_007_rec", addrAg, arrAgentMasterInfo,  MAX_CAP);

  std::ifstream is(filename.c_str());
  pNewAgent->recover(is);
  is.close();
  std::cout<<"----------------The Agent's content after recovering is:----------------"<<std::endl;

  //pNewAgent->print();
  std::cout<<std::endl<<"----------------End  testAgentSerialization----------------"<<std::endl;
}

BOOST_AUTO_TEST_CASE(testAgentSerializationDummyWfe)
{
  std::cout<<std::endl<<"----------------Begin  testAgentSerialization----------------"<<std::endl;
  std::string filename = "testSerializeAgent.txt"; // = boost::archive::tmpdir());filename += "/testfile";

  LOG(INFO, "Test the co-allocation ...");

  string addrAg = "127.0.0.1";
  string strBackupOrch;
  ostringstream oss;

  sdpa::master_info_list_t arrAgentMasterInfo;
  sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<DummyWorkflowEngine>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

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

  std::filebuf fb;
  fb.open (filename.c_str(),std::ios::out);
  std::ostream bkpos(&fb);
  pAgent->backup(bkpos);

  sdpa::daemon::Agent::ptr_t pNewAgent = sdpa::daemon::AgentFactory<DummyWorkflowEngine>::create("agent_007_rec", addrAg, arrAgentMasterInfo,  MAX_CAP);

  std::ifstream is(filename.c_str());
  pNewAgent->recover(is);
  is.close();
  std::cout<<"----------------The Agent's content after recovering is:----------------"<<std::endl;

  //pNewAgent->print();

  std::cout<<std::endl<<"----------------End  testAgentSerialization----------------"<<std::endl;
}

BOOST_AUTO_TEST_CASE(testSchedulerSerialization)
{
  LOG(INFO, "Test the load-balancing when a worker joins later ...");
  std::string filename = "testSerializeAgent.txt";

  string addrAg = "127.0.0.1";
  sdpa::master_info_list_t arrAgentMasterInfo;
  sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo,  MAX_CAP);

  ostringstream oss;
  pAgent-> createScheduler(false);

  sdpa::daemon::Scheduler::ptr_t ptrScheduler = boost::dynamic_pointer_cast<sdpa::daemon::AgentScheduler>(pAgent->scheduler());

  if(!ptrScheduler)
    LOG(FATAL, "The scheduler was not properly initialized");


  // number of workers
  const int nWorkers = 10;
  const int nJobs = 15;

  // create a give number of workers with different capabilities:
  std::ostringstream osstr;
  std::vector<sdpa::worker_id_t> arrWorkerIds;
  for(int k=0;k<nWorkers-1;k++)
  {
    osstr<<"worker_"<<k;
    sdpa::worker_id_t workerId(osstr.str());
    osstr.str("");
    arrWorkerIds.push_back(workerId);
    std::vector<sdpa::capability_t> arrCpbs(1, sdpa::capability_t("C", "virtual", workerId));
    sdpa::capabilities_set_t cpbSet(arrCpbs.begin(), arrCpbs.end());
    ptrScheduler->addWorker(workerId, 1, cpbSet);
  }

  // submit a bunch of jobs now
  std::vector<sdpa::job_id_t> arrJobIds;
  for(int i=0;i<nJobs;i++)
  {
    osstr<<"job_"<<i;
    sdpa::job_id_t jobId(osstr.str());
    arrJobIds.push_back(jobId);
    osstr.str("");
    sdpa::daemon::Job::ptr_t pJob(new JobFSM(jobId, ""));
    pAgent->jobManager()->addJob(jobId, pJob);
    pAgent->jobManager()->addJobRequirements(jobId, job_requirements_t(requirement_list_t(1, requirement_t("C", true)), schedule_data(1, 100)));
  }

  // schedule all jobs now
  BOOST_FOREACH(const sdpa::job_id_t& jobId, arrJobIds)
  {
    ptrScheduler->schedule_remotely(jobId);
  }

  ptrScheduler->assignJobsToWorkers();

  std::cout<<"Scheduler dump before serialzation: "<<std::endl;
  ptrScheduler->print();

  // serialize now the job queue
  try
  {
     std::ofstream ofs(filename.c_str());
     boost::archive::text_oarchive oa(ofs);
     oa.register_type(static_cast<AgentScheduler*>(NULL));
     oa << ptrScheduler;
  }
  catch(exception &e)
  {
      cout <<"Exception occurred: "<< e.what() << endl;
      return;
  }

  // restore state to one equivalent to the original
  try
  {
    Scheduler::ptr_t pSchedRest;
    // open the archive
    std::ifstream ifs(filename.c_str());
    boost::archive::text_iarchive ia(ifs);
    ia.register_type(static_cast<AgentScheduler*>(NULL));
    // restore the schedule from the archive
    ia >> pSchedRest;
    std::cout<<"Scheduler dump after serialzation: "<<std::endl;
    pSchedRest->print();
  }
  catch(exception &e)
  {
      cout <<"Exception occurred: "<< e.what() << endl;
      return;
  }
}

BOOST_AUTO_TEST_CASE(testSynchQueueSerialization)
{
  std::string filename = "testSerializeSynchQueue.txt"; // = boost::archive::tmpdir());filename += "/testfile";

  Worker::JobQueue jobQueue;
  for( int k=0; k<5; k++)
  {
    std::ostringstream ossJobId;;
    ossJobId<<"Job_"<<k;
    sdpa::job_id_t jobId(ossJobId.str());
    jobQueue.push(jobId);
  }

  // serialize now the job queue
  {
     std::ofstream ofs(filename.c_str());
     boost::archive::text_oarchive oa(ofs);
     //oa.register_type(static_cast<Derived *>(NULL));
     oa << jobQueue;
  }

  // restore state to one equivalent to the original
  Worker::JobQueue jobQueueRestored;
  {
     // open the archive
     std::ifstream ifs(filename.c_str());
     boost::archive::text_iarchive ia(ifs);
     //ia.register_type(static_cast<Derived *>(NULL));
     // restore the schedule from the archive
     ia >> jobQueueRestored;
  }

  // print the values of the restored job queue
  cout<<"Restored values of the tested synchronized queue:"<<std::endl;
  for( int k=0; k<5; k++)
  {
      sdpa::job_id_t jobIdRestored = jobQueueRestored.pop();
      cout<<jobIdRestored.str()<<std::endl;
  }
}

BOOST_AUTO_TEST_SUITE_END()
