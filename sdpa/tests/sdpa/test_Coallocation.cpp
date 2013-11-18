#define BOOST_TEST_MODULE testCoallocation

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE(testCollocSched)
{
  LOG(INFO, "Test the co-allocation ...");

  const int NWORKERS = 12;
  const std::string WORKER_CPBS[] = {"A", "B", "C"};

  std::string addrAg = "127.0.0.1";
  std::string strBackupOrch;
  std::ostringstream oss;

  sdpa::master_info_list_t arrAgentMasterInfo;
  sdpa::daemon::Agent::ptr_t pAgent = sdpa::daemon::AgentFactory<void>::create("agent_007", addrAg, arrAgentMasterInfo);

  pAgent->createScheduler();

  sdpa::daemon::CoallocationScheduler* ptrScheduler = dynamic_cast<sdpa::daemon::CoallocationScheduler*>(pAgent->scheduler().get());

  if(!ptrScheduler)
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
    ptrScheduler->addWorker(workerId, 1, cpbSet);
  }

  // create a number of jobs
  const sdpa::job_id_t jobId0("Job0");
  sdpa::daemon::Job::ptr_t pJob0(new sdpa::daemon::Job(jobId0, "description 0", sdpa::job_id_t()));
  job_requirements_t jobReqs0(requirement_list_t(1, requirement_t(WORKER_CPBS[0], true)), schedule_data(4, 100));
  pAgent->addJob(jobId0, pJob0, jobReqs0);

  const sdpa::job_id_t jobId1("Job1");
  sdpa::daemon::Job::ptr_t pJob1(new sdpa::daemon::Job(jobId1, "description 1", sdpa::job_id_t()));
  job_requirements_t jobReqs1(requirement_list_t(1, requirement_t(WORKER_CPBS[1], true)), schedule_data(4, 100));
  pAgent->addJob(jobId1, pJob1, jobReqs1);

  const sdpa::job_id_t jobId2("Job2");
  sdpa::daemon::Job::ptr_t pJob2(new sdpa::daemon::Job(jobId2, "description 2", sdpa::job_id_t()));
  job_requirements_t jobReqs2(requirement_list_t(1, requirement_t(WORKER_CPBS[2], true)), schedule_data(4, 100));
  pAgent->addJob(jobId2, pJob2, jobReqs2);

  ptrScheduler->schedule_remotely(jobId0);
  ptrScheduler->schedule_remotely(jobId1);
  ptrScheduler->schedule_remotely(jobId2);

  ptrScheduler->assignJobsToWorkers();

  std::ostringstream ossrw;int k=-1;
  sdpa::worker_id_list_t listJobAssignedWorkers = ptrScheduler->getListAllocatedWorkers(jobId0);
  BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
  {
    k = boost::lexical_cast<int>(wid);
    BOOST_CHECK( k==0 || k==3 || k==6 || k==9);
  }
  LOG(INFO, "The job jobId0 has been allocated the workers "<<listJobAssignedWorkers);

  listJobAssignedWorkers.clear();
  listJobAssignedWorkers = ptrScheduler->getListAllocatedWorkers(jobId1);
  BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
  {
    k = boost::lexical_cast<int>(wid);
    BOOST_CHECK( k==1 || k==4 || k==7 || k==10);
  }
  LOG(INFO, "The job jobId1 has been allocated the workers "<<listJobAssignedWorkers);

  listJobAssignedWorkers.clear();
  listJobAssignedWorkers = ptrScheduler->getListAllocatedWorkers(jobId2);
  BOOST_FOREACH(sdpa::worker_id_t& wid, listJobAssignedWorkers)
  {
    k = boost::lexical_cast<int>(wid);
    BOOST_CHECK( k==2 || k==5 || k==8 || k==11);
  }
  LOG(INFO, "The job jobId2 has been allocated the workers "<<listJobAssignedWorkers);

  // try now to schedule a job requiring 2 resources of type "A"
  const sdpa::job_id_t jobId4("Job4");
  sdpa::daemon::Job::ptr_t pJob4(new sdpa::daemon::Job(jobId4, "description 4", sdpa::job_id_t()));
  job_requirements_t jobReqs4(requirement_list_t(1, requirement_t(WORKER_CPBS[0], true)), schedule_data(2, 100));
  pAgent->addJob(jobId4, pJob4, jobReqs4);

  ptrScheduler->schedule_remotely(jobId4);

  ptrScheduler->assignJobsToWorkers();
  sdpa::worker_id_list_t listFreeWorkers(ptrScheduler->getListAllocatedWorkers(jobId4));
  BOOST_CHECK(listFreeWorkers.empty());

  //reinterpret_cast<SchedulerBase*>(ptrScheduler.get())->printAllocationTable();

  // Now report that jobId0 has finished and try to assign again resources to the job 4
  ptrScheduler->releaseReservation(jobId0);

  //listFreeWorkers.clear();
  ptrScheduler->assignJobsToWorkers();

  listFreeWorkers = ptrScheduler->getListAllocatedWorkers(jobId4);
  BOOST_CHECK(!listFreeWorkers.empty());

  int w0 = boost::lexical_cast<int>(listFreeWorkers.front());
  BOOST_CHECK(w0==0 || w0 == 3  || w0 == 6|| w0 == 9);

  //int w1 = boost::lexical_cast<int>(*(boost::next(listFreeWorkers.begin())));
  //BOOST_CHECK(w1==0 || w1 == 3  || w1 == 6|| w1 == 9);
}

BOOST_AUTO_TEST_CASE (testCoallocationWorkflow)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent<we::mgmt::layer> agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_A_0
    ( "drts_A_0", agent
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_A_1
    ( "drts_A_1", agent
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  const utils::drts_worker worker_B_0
    ( "drts_B_0", agent
    , "B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_B_1
    ( "drts_B_1", agent
    , "B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_B_2
    ( "drts_B_2", agent
    , "B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  utils::client::submit_job_and_wait_for_termination_as_subscriber
    (workflow, "sdpac", orchestrator);
}
