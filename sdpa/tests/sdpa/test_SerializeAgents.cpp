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

#include <sdpa/engine/DummyWorkflowEngine.hpp>
#include <tests/sdpa/CreateDrtsWorker.hpp>

#include "kvs_setup_fixture.hpp"
BOOST_GLOBAL_FIXTURE (KVSSetup);

const int MAX_CAP = 100;

using namespace sdpa::tests;
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

BOOST_AUTO_TEST_SUITE( test_SerializeAgents );

BOOST_AUTO_TEST_CASE(testDummyWorkflowEngineSerialization)
{
	std::cout<<std::endl<<"----------------Begin  testDummyWorkflowEngineSerialization----------------"<<std::endl;

	std::string filename = "testDummyWorkflowEngineSerialization.txt"; // = boost::archive::tmpdir());filename += "/testfile";

	IWorkflowEngine* pWfEng = new DummyWorkflowEngine();

	const int NWFS = 10;
	encoded_type wf_desc("workflow description ");

	for(int i=0; i<NWFS; i++)
	{
		sdpa::job_id_t jobId;
		pWfEng->submit(jobId.str(), wf_desc);
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
		IWorkflowEngine* pRestoredWfEng;
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

/*
BOOST_AUTO_TEST_CASE(testAgentSerialization)
{
    std::cout<<std::endl<<"----------------Begin  testAgentSerialization----------------"<<std::endl;
    std::string filename = "testSerializeAgent.txt"; // = boost::archive::tmpdir());filename += "/testfile";
    Agent::ptr_t pAgg = sdpa::daemon::AgentFactory<DummyWorkflowEngine>::create(	"agent_0",
    																						"127.0.0.1:7001",
    																						sdpa::master_info_list_t(1,MasterInfo("orchestrator_0")),
    																						MAX_CAP); //, "127.0.0.1:7000");

    pAgg->setScheduler(new SchedulerImpl());
    SchedulerImpl* pScheduler = dynamic_cast<SchedulerImpl*>(pAgg->scheduler().get());

    JobId id1("_1");
    sdpa::daemon::Job::ptr_t  p1(new JobFSM(id1, "decsription 1"));
    pAgg->jobManager()->addJob(id1, p1);

    JobId id2("_2");
    sdpa::daemon::Job::ptr_t  p2(new JobFSM(id2, "decsription 2"));
    pAgg->jobManager()->addJob(id2, p2);

    JobId id3("_3");
    sdpa::daemon::Job::ptr_t  p3(new JobFSM(id3, "decsription 3"));
    pAgg->jobManager()->addJob(id3, p3);

    JobId id4("_4");
    sdpa::daemon::Job::ptr_t  p4(new JobFSM(id4, "decsription 4"));
    pAgg->jobManager()->addJob(id4, p4);

    JobId id5("_5");
    sdpa::daemon::Job::ptr_t  p5(new JobFSM(id5, "decsription 5"));
    pAgg->jobManager()->addJob(id5, p5);

    int nSchedQSize = 5;
    for(int i=0; i<nSchedQSize; i++)
    {
      std::ostringstream ossJobId;;
      ossJobId<<"Job_"<<i;
      sdpa::job_id_t jobId(ossJobId.str());
      pScheduler->schedule(jobId);
    }

    int nWorkers=3;
    for(int k=0; k<nWorkers; k++)
    {
      std::ostringstream ossWorkerId;;
      ossWorkerId<<"Worker_"<<k;
      Worker::worker_id_t workerId(ossWorkerId.str());
      pScheduler->addWorker(workerId, k);

      for( int l=0; l<3; l++)
      {
        std::ostringstream ossJobId;
        ossJobId<<"Job_"<<k*nWorkers + l + nSchedQSize;
        sdpa::job_id_t jobId(ossJobId.str());

        pScheduler->schedule_to(jobId, workerId);
        if(l>=1)
        {
          sdpa::job_id_t jobToSubmit = pScheduler->getNextJob(workerId, "");
          if(l>=2)
                  pScheduler->acknowledgeJob(workerId, jobToSubmit);
        }
      }
    }

    try
    {
      std::cout<<"----------------The Agent<DummyWorkflowEngine>'s content before backup is:----------------"<<std::endl;
      pAgg->print();

      std::ofstream ofs(filename.c_str());
      boost::archive::text_oarchive oa(ofs);
      oa.register_type(static_cast<Agent*>(NULL));
      oa.register_type(static_cast<DummyWorkflowEngine*>(NULL));
      oa.register_type(static_cast<DaemonFSM*>(NULL));
      oa.register_type(static_cast<GenericDaemon*>(NULL));
      oa.register_type(static_cast<SchedulerImpl*>(NULL));
      oa.register_type(static_cast<JobFSM*>(NULL));
      oa << pAgg;
    }
    catch(exception &e)
    {
      cout <<"Exception occurred: "<< e.what() << endl;
      return;
    }

    std::cout<<"----------------Try now to restore the Agent<DummyWorkflowEngine>:----------------"<<std::endl;
    try
    {
      Agent::ptr_t pRestoredAgg;
      std::ifstream ifs(filename.c_str());
      boost::archive::text_iarchive ia(ifs);
      ia.register_type(static_cast<Agent*>(NULL));
      ia.register_type(static_cast<DummyWorkflowEngine*>(NULL));
      ia.register_type(static_cast<DaemonFSM*>(NULL));
      ia.register_type(static_cast<GenericDaemon*>(NULL));
      ia.register_type(static_cast<SchedulerImpl*>(NULL));
      ia.register_type(static_cast<JobFSM*>(NULL));
      ia >> pRestoredAgg;

      std::cout<<std::endl<<"----------------The restored content of the Agent<DummyWorkflowEngine> is:----------------"<<std::endl;
      pRestoredAgg->print();
    }
    catch(exception &e)
    {
        cout <<"Exception occurred: " << e.what() << endl;
        return;
    }

    std::cout<<std::endl<<"----------------End  testAgentSerialization----------------"<<std::endl;
}

BOOST_AUTO_TEST_CASE(testOrchestratorSerialization)
{
    std::cout<<std::endl<<"----------------Begin  testOrchestratorSerialization----------------"<<std::endl;
    std::string filename = "testSerializeOrchestrator.txt"; // = boost::archive::tmpdir());filename += "/testfile";
    Orchestrator::ptr_t pOrch = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create( "orchestrator_0", "127.0.0.1:6000", MAX_CAP);

    pOrch->setScheduler(new SchedulerImpl());
    SchedulerImpl* pScheduler = dynamic_cast<SchedulerImpl*>(pOrch->scheduler().get());

    JobId id1("_1");
    sdpa::daemon::Job::ptr_t  p1(new JobFSM(id1, "decsription 1"));
    pOrch->jobManager()->addJob(id1, p1);

    JobId id2("_2");
    sdpa::daemon::Job::ptr_t  p2(new JobFSM(id2, "decsription 2"));
    pOrch->jobManager()->addJob(id2, p2);

    JobId id3("_3");
    sdpa::daemon::Job::ptr_t  p3(new JobFSM(id3, "decsription 3"));
    pOrch->jobManager()->addJob(id3, p3);

    JobId id4("_4");
    sdpa::daemon::Job::ptr_t  p4(new JobFSM(id4, "decsription 4"));
    pOrch->jobManager()->addJob(id4, p4);

    JobId id5("_5");
    sdpa::daemon::Job::ptr_t  p5(new JobFSM(id5, "decsription 5"));
    pOrch->jobManager()->addJob(id5, p5);

    int nSchedQSize = 5;
    for(int i=0; i<nSchedQSize; i++)
    {
      std::ostringstream ossJobId;;
      ossJobId<<"Job_"<<i;
      sdpa::job_id_t jobId(ossJobId.str());
      pScheduler->schedule(jobId);
    }

    int nWorkers=3;
    for(int k=0; k<nWorkers; k++)
    {
      std::ostringstream ossWorkerId;;
      ossWorkerId<<"Worker_"<<k;
      Worker::worker_id_t workerId(ossWorkerId.str());
      pScheduler->addWorker(workerId, k);

      for( int l=0; l<3; l++)
      {
          std::ostringstream ossJobId;
          ossJobId<<"Job_"<<k*nWorkers + l + nSchedQSize;
          sdpa::job_id_t jobId(ossJobId.str());

          pScheduler->schedule_to(jobId, workerId);
          if(l>=1)
          {
              sdpa::job_id_t jobToSubmit = pScheduler->getNextJob(workerId, "");
              if(l>=2)
                pScheduler->acknowledgeJob(workerId, jobToSubmit);
          }
        }
    }

    try
    {
        std::cout<<"----------------The Orchestrator's content before backup is:----------------"<<std::endl;
        pOrch->print();

        std::ofstream ofs(filename.c_str());
        boost::archive::text_oarchive oa(ofs);
        oa.register_type(static_cast<Orchestrator*>(NULL));
        oa.register_type(static_cast<DummyWorkflowEngine*>(NULL));
        oa.register_type(static_cast<DaemonFSM*>(NULL));
        oa.register_type(static_cast<GenericDaemon*>(NULL));
        oa.register_type(static_cast<SchedulerImpl*>(NULL));
        oa.register_type(static_cast<SchedulerOrch*>(NULL));
        oa.register_type(static_cast<JobFSM*>(NULL));
        oa << pOrch;
    }
    catch(exception &e)
    {
        cout <<"Exception occurred: "<< e.what() << endl;
        return;
    }

    std::cout<<"----------------Try now to restore the orchestrator:----------------"<<std::endl;
    try
    {
        Orchestrator::ptr_t pRestoredOrch;
        std::ifstream ifs(filename.c_str());
        boost::archive::text_iarchive ia(ifs);
        ia.register_type(static_cast<Orchestrator*>(NULL));
        ia.register_type(static_cast<DummyWorkflowEngine*>(NULL));
        ia.register_type(static_cast<DaemonFSM*>(NULL));
        ia.register_type(static_cast<GenericDaemon*>(NULL));
        ia.register_type(static_cast<SchedulerImpl*>(NULL));
        ia.register_type(static_cast<SchedulerOrch*>(NULL));
        ia.register_type(static_cast<JobFSM*>(NULL));
        ia >> pRestoredOrch;

        std::cout<<std::endl<<"----------------The restored content of the Orchestrator is:----------------"<<std::endl;
        pRestoredOrch->print();
    }
    catch(exception &e)
    {
        cout <<"Exception occurred: " << e.what() << endl;
        return;
    }

    std::cout<<std::endl<<"----------------End  testOrchestratorSerialization----------------"<<std::endl;
}

BOOST_AUTO_TEST_CASE(testDaemonSerializationWithFSMs)
{
  std::string filename = "testSerializeDaemons.txt"; // = boost::archive::tmpdir());filename += "/testfile";
  GenericDaemon::ptr_t pGenDaemon( new GenericDaemon( sdpa::daemon::ORCHESTRATOR ));

  pGenDaemon->setScheduler( new SchedulerImpl() );
  SchedulerImpl* pScheduler = dynamic_cast<SchedulerImpl*>(pGenDaemon->scheduler().get());

  // First fill-in the JobManager ...
  JobId id1("_1");
  sdpa::daemon::Job::ptr_t  p1(new JobFSM(id1, "decsription 1"));
  pGenDaemon->jobManager()->addJob(id1, p1);

  JobId id2("_2");
  sdpa::daemon::Job::ptr_t  p2(new JobFSM(id2, "decsription 2"));
  pGenDaemon->jobManager()->addJob(id2, p2);

  JobId id3("_3");
  sdpa::daemon::Job::ptr_t  p3(new JobFSM(id3, "decsription 3"));
  pGenDaemon->jobManager()->addJob(id3, p3);

  JobId id4("_4");
  sdpa::daemon::Job::ptr_t  p4(new JobFSM(id4, "decsription 4"));
  pGenDaemon->jobManager()->addJob(id4, p4);

  JobId id5("_5");
  sdpa::daemon::Job::ptr_t  p5(new JobFSM(id5, "decsription 5"));
  pGenDaemon->jobManager()->addJob(id5, p5);


  /// And then, the scheduler
  int nSchedQSize = 5;
  for(int i=0; i<nSchedQSize; i++)
  {
      std::ostringstream ossJobId;;
      ossJobId<<"Job_"<<i;
      sdpa::job_id_t jobId(ossJobId.str());
      pScheduler->schedule(jobId);
  }

  int nWorkers=3;
  for(int k=0; k<nWorkers; k++)
  {
      std::ostringstream ossWorkerId;;
      ossWorkerId<<"Worker_"<<k;
      Worker::worker_id_t workerId(ossWorkerId.str());
      pScheduler->addWorker(workerId, k);

      for( int l=0; l<3; l++)
      {
          std::ostringstream ossJobId;
          ossJobId<<"Job_"<<k*nWorkers + l + nSchedQSize;
          sdpa::job_id_t jobId(ossJobId.str());

          pScheduler->schedule_to(jobId, workerId);
          if(l>=1)
          {
                  sdpa::job_id_t jobToSubmit = pScheduler->getNextJob(workerId, "");
                  if(l>=2)
                          pScheduler->acknowledgeJob(workerId, jobToSubmit);
          }
      }
  }

  try
  {
      std::cout<<"----------------The daemon's content before backup is:----------------"<<std::endl;
      pGenDaemon->print();

      std::ofstream ofs(filename.c_str());
      boost::archive::text_oarchive oa(ofs);
      oa.register_type(static_cast<SchedulerImpl*>(NULL));
      oa.register_type(static_cast<JobFSM*>(NULL));
      oa << pGenDaemon;
  }
  catch(exception &e)
  {
      cout <<"Exception occurred: "<< e.what() << endl;
      return;
  }

  // de-serialize now the daemon
  try
  {
      GenericDaemon::ptr_t pRestoredGenDaemon;
      std::ifstream ifs(filename.c_str());
      boost::archive::text_iarchive ia(ifs);
      ia.register_type(static_cast<SchedulerImpl*>(NULL));
      ia.register_type(static_cast<JobFSM*>(NULL));
      ia >> pRestoredGenDaemon;

      std::cout<<std::endl<<"----------------The restored content of the daemon is:----------------"<<std::endl;
      pRestoredGenDaemon->print();
  }
  catch(exception &e)
  {
      cout <<"Exception occurred: " << e.what() << endl;
      return;
  }
}

BOOST_AUTO_TEST_CASE(testDaemonSerialization)
{
  std::string filename = "testSerializeDaemons.txt"; // = boost::archive::tmpdir());filename += "/testfile";
  GenericDaemon::ptr_t pGenDaemon( new GenericDaemon( sdpa::daemon::ORCHESTRATOR ));

  pGenDaemon->setScheduler(new SchedulerImpl());
  SchedulerImpl* pScheduler = dynamic_cast<SchedulerImpl*>(pGenDaemon->scheduler().get());

  // First fill-in the JobManager ...
  JobId id1("_1");
  sdpa::daemon::Job::ptr_t  p1(new sdpa::daemon::JobImpl(id1, "decsription 1"));
  pGenDaemon->jobManager()->addJob(id1, p1);

  JobId id2("_2");
  sdpa::daemon::Job::ptr_t  p2(new sdpa::daemon::JobImpl(id2, "decsription 2"));
  pGenDaemon->jobManager()->addJob(id2, p2);

  JobId id3("_3");
  sdpa::daemon::Job::ptr_t  p3(new sdpa::daemon::JobImpl(id3, "decsription 3"));
  pGenDaemon->jobManager()->addJob(id3, p3);

  JobId id4("_4");
  sdpa::daemon::Job::ptr_t  p4(new sdpa::daemon::JobImpl(id4, "decsription 4"));
  pGenDaemon->jobManager()->addJob(id4, p4);

  JobId id5("_5");
  sdpa::daemon::Job::ptr_t  p5(new sdpa::daemon::JobImpl(id5, "decsription 5"));
  pGenDaemon->jobManager()->addJob(id5, p5);


  /// And then, the scheduler
  int nSchedQSize = 5;
  for(int i=0; i<nSchedQSize; i++)
  {
    std::ostringstream ossJobId;;
    ossJobId<<"Job_"<<i;
    sdpa::job_id_t jobId(ossJobId.str());
    pScheduler->schedule(jobId);
  }

  int nWorkers=3;
  for(int k=0; k<nWorkers; k++)
  {
    std::ostringstream ossWorkerId;;
    ossWorkerId<<"Worker_"<<k;
    Worker::worker_id_t workerId(ossWorkerId.str());
    pScheduler->addWorker(workerId, k);

    for( int l=0; l<3; l++)
    {
        std::ostringstream ossJobId;
        ossJobId<<"Job_"<<k*nWorkers + l + nSchedQSize;
        sdpa::job_id_t jobId(ossJobId.str());

        pScheduler->schedule_to(jobId, workerId);
        if(l>=1)
        {
                sdpa::job_id_t jobToSubmit = pScheduler->getNextJob(workerId, "");
                if(l>=2)
                        pScheduler->acknowledgeJob(workerId, jobToSubmit);
        }
    }
  }

  try
  {
      std::cout<<std::endl<<"----------------The daemon's content before backup is:----------------"<<std::endl;
      pGenDaemon->print();

      std::ofstream ofs(filename.c_str());
      boost::archive::text_oarchive oa(ofs);
      oa.register_type(static_cast<SchedulerImpl*>(NULL));
      oa.register_type(static_cast<JobImpl*>(NULL));
      oa << pGenDaemon;
  }
  catch(exception &e)
  {
      cout <<"Exception occurred: "<< e.what() << endl;
      return;
  }

  // de-serialize now the daemon
  try
  {
      GenericDaemon::ptr_t pRestoredGenDaemon;
      std::ifstream ifs(filename.c_str());
      boost::archive::text_iarchive ia(ifs);
      ia.register_type(static_cast<SchedulerImpl*>(NULL));
      ia.register_type(static_cast<JobImpl*>(NULL));
      ia >> pRestoredGenDaemon;

      std::cout<<"----------------The restored content of the daemon is:----------------"<<std::endl;
      pRestoredGenDaemon->print();
  }
  catch(exception &e)
  {
      cout <<"Exception occurred: " << e.what() << endl;
      return;
  }
}

BOOST_AUTO_TEST_CASE(testSchedulerSerialization)
{
  std::string filename = "testSerializeScheduler.txt"; // = boost::archive::tmpdir());filename += "/testfile";
  Scheduler::ptr_t pScheduler(new SchedulerImpl());

  int nSchedQSize = 5;
  for(int i=0; i<nSchedQSize; i++)
  {
      std::ostringstream ossJobId;;
      ossJobId<<"Job_"<<i;
      sdpa::job_id_t jobId(ossJobId.str());
      dynamic_cast<SchedulerImpl*>(pScheduler.get())->schedule(jobId);
  }

  int nWorkers=3;
  for(int k=0; k<nWorkers; k++)
  {
    std::ostringstream ossWorkerId;;
    ossWorkerId<<"Worker_"<<k;
    Worker::worker_id_t workerId(ossWorkerId.str());
    pScheduler->addWorker(workerId, k);

    for( int l=0; l<3; l++)
    {
        std::ostringstream ossJobId;
        ossJobId<<"Job_"<<k*nWorkers + l + nSchedQSize;
        sdpa::job_id_t jobId(ossJobId.str());

        pScheduler->schedule_to(jobId, workerId);
        if(l>=1)
        {
                sdpa::job_id_t jobToSubmit = pScheduler->getNextJob(workerId, "");
                if(l>=2)
                        pScheduler->acknowledgeJob(workerId, jobToSubmit);
        }
    }
  }

  std::cout<<"Scheduler dump before serialzation: "<<std::endl;
  pScheduler->print();

  // serialize now the job queue
  try
  {
     std::ofstream ofs(filename.c_str());
     boost::archive::text_oarchive oa(ofs);
     oa.register_type(static_cast<SchedulerImpl*>(NULL));
     oa << pScheduler;
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
      ia.register_type(static_cast<SchedulerImpl*>(NULL));
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

BOOST_AUTO_TEST_CASE(testWorkerSerialization)
{

    std::string filename = "testSerializeWorker.txt"; // = boost::archive::tmpdir());filename += "/testfile";

    Worker testWorker("testWorker");

    for( int k=0; k<6; k++)
    {
        std::ostringstream ossJobId;;
        ossJobId<<"Job_"<<k;
        sdpa::job_id_t jobId(ossJobId.str());
        testWorker.dispatch(jobId);
        if(k>=2)
        {
            sdpa::job_id_t jobToSubmit = testWorker.get_next_job("");
            if(k>=4)
              testWorker.acknowledge(jobToSubmit);
        }
    }

    cout<<"Worker job ids to serialize:"<<std::endl;
    testWorker.print();

    // serialize now the job queue
    {
       std::ofstream ofs(filename.c_str());
       boost::archive::text_oarchive oa(ofs);
       //oa.register_type(static_cast<Derived *>(NULL));
       oa << testWorker;
    }

    // restore state to one equivalent to the original
    {
            Worker workerRestored("restoredWorker");
       // open the archive
       std::ifstream ifs(filename.c_str());
       boost::archive::text_iarchive ia(ifs);
       //ia.register_type(static_cast<Derived *>(NULL));
       // restore the schedule from the archive
       ia >> workerRestored;

       cout<<"Restored worker values:"<<std::endl;
       workerRestored.print();
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
*/

BOOST_AUTO_TEST_SUITE_END()
