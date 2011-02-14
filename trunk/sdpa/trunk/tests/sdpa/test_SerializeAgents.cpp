#define BOOST_TEST_MODULE TestAgentsSerialization

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

#include <sdpa/engine/DummyWorkflowEngine.hpp>
#include <sdpa/engine/EmptyWorkflowEngine.hpp>
#include <sdpa/engine/RealWorkflowEngine.hpp>

#include <sdpa/client/ClientApi.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/Strategy.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>
#include <boost/thread.hpp>

using namespace sdpa::tests;
using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

typedef sdpa::nre::worker::NreWorkerClient WorkerClient;

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("12100"); return s; }

struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
	{ //initialize and start the finite state machine

		FHGLOG_SETUP();

		LOG(DEBUG, "Fixture's constructor called ...");

		m_ptrPool = new fhg::com::io_service_pool(1);
		m_ptrKvsd = new fhg::com::kvs::server::kvsd ("/tmp/notthere");
		m_ptrServ = new fhg::com::tcp_server ( *m_ptrPool
										  , *m_ptrKvsd
										  , kvs_host ()
										  , kvs_port ()
										  , true
										  );

		m_ptrThrd = new boost::thread (boost::bind ( &fhg::com::io_service_pool::run
												, m_ptrPool
												)
								   );

		m_ptrServ->start();
		sleep (1);

		fhg::com::kvs::get_or_create_global_kvs ( kvs_host()
												, kvs_port()
												, true
												, boost::posix_time::seconds(10)
												, 3
												);
	}

	~MyFixture()
	{
		LOG(DEBUG, "Fixture's destructor called ...");
		//stop the finite state machine

		m_ptrPool->stop ();
		m_ptrThrd->join ();
		delete m_ptrThrd;
		delete m_ptrServ;
		delete m_ptrKvsd;
		delete m_ptrPool;
	}

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

	fhg::com::io_service_pool *m_ptrPool;
	fhg::com::kvs::server::kvsd *m_ptrKvsd;
	fhg::com::tcp_server *m_ptrServ;
	boost::thread *m_ptrThrd;
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

BOOST_AUTO_TEST_CASE(testNRESerialization)
{
	std::cout<<std::endl<<"----------------Begin  testNRESerialization----------------"<<std::endl;
	std::string filename = "testSerializeNRE.txt"; // = boost::archive::tmpdir());filename += "/testfile";
	sdpa::daemon::NRE<WorkerClient>::ptr_t ptrNRE_0
          (
           sdpa::daemon::NREFactory< DummyWorkflowEngine
                            , WorkerClient
                            >::create( "NRE_0"
                                     , "127.0.0.1:7002"
                                     , "aggregator_0"
                                     , "127.0.0.1:7001"
                                     , "127.0.0.1:8000"
                                     )
          );

	ptrNRE_0->setScheduler( new sdpa::daemon::SchedulerNRE<WorkerClient>() );
	//sdpa::daemon::SchedulerImpl* pScheduler = dynamic_cast< sdpa::daemon::SchedulerImpl*>(ptrNRE_0->scheduler().get());

	JobId id1("_1");
	sdpa::daemon::Job::ptr_t  p1(new JobFSM(id1, "decsription 1"));
	ptrNRE_0->jobManager()->addJob(id1, p1);

	JobId id2("_2");
	sdpa::daemon::Job::ptr_t  p2(new JobFSM(id2, "decsription 2"));
	ptrNRE_0->jobManager()->addJob(id2, p2);

	JobId id3("_3");
	sdpa::daemon::Job::ptr_t  p3(new JobFSM(id3, "decsription 3"));
	ptrNRE_0->jobManager()->addJob(id3, p3);

	JobId id4("_4");
	sdpa::daemon::Job::ptr_t  p4(new JobFSM(id4, "decsription 4"));
	ptrNRE_0->jobManager()->addJob(id4, p4);

	JobId id5("_5");
	sdpa::daemon::Job::ptr_t  p5(new JobFSM(id5, "decsription 5"));
	ptrNRE_0->jobManager()->addJob(id5, p5);

	int nSchedQSize = 5;
	for(int i=0; i<nSchedQSize; i++)
	{
		std::ostringstream ossJobId;;
		ossJobId<<"Job_"<<i;
		sdpa::job_id_t jobId(ossJobId.str());
		ptrNRE_0->scheduler()->schedule(jobId);
	}

	try
	{
		std::cout<<"----------------The NRE's content before backup is:----------------"<<std::endl;
		//ptrNRE_0->print();

		std::ofstream ofs(filename.c_str());
		boost::archive::text_oarchive oa(ofs);
		//oa.register_type(static_cast<NRE<DummyWorkflowEngine, WorkerClient>*>(NULL));
		oa.register_type(static_cast<DummyWorkflowEngine*>(NULL));
		oa.register_type(static_cast<DaemonFSM*>(NULL));
		oa.register_type(static_cast<GenericDaemon*>(NULL));
		oa.register_type(static_cast<SchedulerImpl*>(NULL));
		oa.register_type(static_cast<SchedulerNRE<WorkerClient>*>(NULL));
		oa.register_type(static_cast<JobFSM*>(NULL));

		oa << ptrNRE_0;
	}
	catch(exception &e)
	{
		cout <<"Exception occurred: "<< e.what() << endl;
		return;
	}

	std::cout<<"----------------Try now to restore the NRE:----------------"<<std::endl;
	try
	{
		sdpa::daemon::NRE<WorkerClient>::ptr_t ptrRestoredNRE_0;

		std::ifstream ifs(filename.c_str());
		boost::archive::text_iarchive ia(ifs);
		//ia.register_type(static_cast<NRE<DummyWorkflowEngine, WorkerClient>*>(NULL));
		ia.register_type(static_cast<DummyWorkflowEngine*>(NULL));
		ia.register_type(static_cast<DaemonFSM*>(NULL));
		ia.register_type(static_cast<GenericDaemon*>(NULL));
		ia.register_type(static_cast<SchedulerImpl*>(NULL));
		ia.register_type(static_cast<SchedulerNRE<WorkerClient>*>(NULL));
		ia.register_type(static_cast<JobFSM*>(NULL));

		ia >> ptrRestoredNRE_0;

		std::cout<<std::endl<<"----------------The restored content of the NRE is:----------------"<<std::endl;
		ptrRestoredNRE_0->print();
	}
	catch(exception &e)
	{
		cout <<"Exception occurred: " << e.what() << endl;
		return;
	}

	std::cout<<std::endl<<"----------------End  NRE----------------"<<std::endl;

}

BOOST_AUTO_TEST_CASE(testAggregatorSerialization)
{
	std::cout<<std::endl<<"----------------Begin  testAggregatorSerialization----------------"<<std::endl;
	std::string filename = "testSerializeAggregator.txt"; // = boost::archive::tmpdir());filename += "/testfile";
	Aggregator::ptr_t pAgg = sdpa::daemon::AggregatorFactory<DummyWorkflowEngine>::create("aggregator_0", "127.0.0.1:7001","orchestrator_0"); //, "127.0.0.1:7000");

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

			const preference_t job_pref;
			pScheduler->schedule_to(jobId, k, job_pref);
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
		std::cout<<"----------------The Aggregator<DummyWorkflowEngine>'s content before backup is:----------------"<<std::endl;
		pAgg->print();

		std::ofstream ofs(filename.c_str());
		boost::archive::text_oarchive oa(ofs);
		oa.register_type(static_cast<Aggregator*>(NULL));
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

	std::cout<<"----------------Try now to restore the Aggregator<DummyWorkflowEngine>:----------------"<<std::endl;
	try
	{
		Aggregator::ptr_t pRestoredAgg;
		std::ifstream ifs(filename.c_str());
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<Aggregator*>(NULL));
		ia.register_type(static_cast<DummyWorkflowEngine*>(NULL));
		ia.register_type(static_cast<DaemonFSM*>(NULL));
		ia.register_type(static_cast<GenericDaemon*>(NULL));
		ia.register_type(static_cast<SchedulerImpl*>(NULL));
		ia.register_type(static_cast<JobFSM*>(NULL));
		ia >> pRestoredAgg;

		std::cout<<std::endl<<"----------------The restored content of the Aggregator<DummyWorkflowEngine> is:----------------"<<std::endl;
		pRestoredAgg->print();
	}
	catch(exception &e)
	{
		cout <<"Exception occurred: " << e.what() << endl;
		return;
	}

	std::cout<<std::endl<<"----------------End  testAggregatorSerialization----------------"<<std::endl;
}


BOOST_AUTO_TEST_CASE(testOrchestratorSerialization)
{
	std::cout<<std::endl<<"----------------Begin  testOrchestratorSerialization----------------"<<std::endl;
	std::string filename = "testSerializeOrchestrator.txt"; // = boost::archive::tmpdir());filename += "/testfile";
	Orchestrator::ptr_t pOrch = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create( "orchestrator_0", "127.0.0.1:6000");

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

			const preference_t job_pref;
			pScheduler->schedule_to(jobId, k, job_pref);
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
	GenericDaemon::ptr_t pGenDaemon( new GenericDaemon( sdpa::daemon::ORCHESTRATOR, NULL ));

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

			const preference_t job_pref;
			pScheduler->schedule_to(jobId, k, job_pref);
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
	GenericDaemon::ptr_t pGenDaemon( new GenericDaemon( sdpa::daemon::ORCHESTRATOR, NULL ));

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

			const preference_t job_pref;
			pScheduler->schedule_to(jobId, k, job_pref);
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

			const preference_t job_pref;
			pScheduler->schedule_to(jobId, k, job_pref);
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

BOOST_AUTO_TEST_SUITE_END()
