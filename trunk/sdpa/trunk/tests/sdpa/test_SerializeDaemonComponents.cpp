#include <iostream>

#include <tests/sdpa/test_SerializeDaemonComponents.hpp>
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
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <sdpa/daemon/JobManager.hpp>

#include <boost/serialization/export.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/orchestrator/SchedulerOrch.hpp>
#include <sdpa/daemon/aggregator/Aggregator.hpp>
#include <sdpa/daemon/nre/NRE.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <sdpa/daemon/nre/SchedulerNRE.hpp>
#include <tests/sdpa/DummyWorkflowEngine.hpp>

#include <sdpa/client/ClientApi.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/Strategy.hpp>

using namespace sdpa::tests;
using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

CPPUNIT_TEST_SUITE_REGISTRATION( sdpa::tests::WorkerSerializationTest );

WorkerSerializationTest::WorkerSerializationTest() {
}

WorkerSerializationTest::~WorkerSerializationTest() {
}

void WorkerSerializationTest::setUp() {
}

void WorkerSerializationTest::tearDown() {
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

void WorkerSerializationTest::testDummyWorkflowEngineSerialization()
{
	std::cout<<std::endl<<"----------------Begin  testDummyWorkflowEngineSerialization----------------"<<std::endl;

	/*
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

*/
	std::cout<<std::endl<<"----------------End  testDummyWorkflowEngineSerialization----------------"<<std::endl;
}

void WorkerSerializationTest::testNRESerialization()
{
	std::cout<<std::endl<<"----------------Begin  testNRESerialization----------------"<<std::endl;
	std::string filename = "testSerializeNRE.txt"; // = boost::archive::tmpdir());filename += "/testfile";
	sdpa::daemon::NRE< DummyWorkflowEngine
                         , sdpa::nre::worker::NreWorkerClient
                         >::ptr_t ptrNRE_0
          (
           sdpa::daemon::NRE< DummyWorkflowEngine
                            , sdpa::nre::worker::NreWorkerClient
                            >::create( "NRE_0"
                                     , "127.0.0.1:7002"
                                     , "aggregator_0"
                                     , "127.0.0.1:7001"
                                     , "127.0.0.1:8000"
                                     )
          );

	ptrNRE_0->ptr_scheduler_ = sdpa::daemon::SchedulerNRE<sdpa::nre::worker::NreWorkerClient>::ptr_t(new sdpa::daemon::SchedulerNRE<sdpa::nre::worker::NreWorkerClient>());
	sdpa::daemon::SchedulerImpl* pScheduler = dynamic_cast< sdpa::daemon::SchedulerImpl*>(ptrNRE_0->ptr_scheduler_.get());

	JobId id1("_1");
	sdpa::daemon::Job::ptr_t  p1(new JobFSM(id1, "decsription 1"));
	ptrNRE_0->ptr_job_man_->addJob(id1, p1);

	JobId id2("_2");
	sdpa::daemon::Job::ptr_t  p2(new JobFSM(id2, "decsription 2"));
	ptrNRE_0->ptr_job_man_->addJob(id2, p2);

	JobId id3("_3");
	sdpa::daemon::Job::ptr_t  p3(new JobFSM(id3, "decsription 3"));
	ptrNRE_0->ptr_job_man_->addJob(id3, p3);

	JobId id4("_4");
	sdpa::daemon::Job::ptr_t  p4(new JobFSM(id4, "decsription 4"));
	ptrNRE_0->ptr_job_man_->addJob(id4, p4);

	JobId id5("_5");
	sdpa::daemon::Job::ptr_t  p5(new JobFSM(id5, "decsription 5"));
	ptrNRE_0->ptr_job_man_->addJob(id5, p5);

	int nSchedQSize = 5;
	for(int i=0; i<nSchedQSize; i++)
	{
		std::ostringstream ossJobId;;
		ossJobId<<"Job_"<<i;
		sdpa::job_id_t jobId(ossJobId.str());
		pScheduler->jobs_to_be_scheduled.push(jobId);
	}

	try
	{
		std::cout<<"----------------The NRE's content before backup is:----------------"<<std::endl;
		//ptrNRE_0->print();

		std::ofstream ofs(filename.c_str());
		boost::archive::text_oarchive oa(ofs);
		oa.register_type(static_cast<NRE<DummyWorkflowEngine, sdpa::nre::worker::NreWorkerClient>*>(NULL));
		oa.register_type(static_cast<DummyWorkflowEngine*>(NULL));
		oa.register_type(static_cast<DaemonFSM*>(NULL));
		oa.register_type(static_cast<GenericDaemon*>(NULL));
		oa.register_type(static_cast<SchedulerImpl*>(NULL));
		oa.register_type(static_cast<SchedulerNRE<sdpa::nre::worker::NreWorkerClient>*>(NULL));
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
		sdpa::daemon::NRE<DummyWorkflowEngine, sdpa::nre::worker::NreWorkerClient>::ptr_t ptrRestoredNRE_0;

		std::ifstream ifs(filename.c_str());
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<NRE<DummyWorkflowEngine, sdpa::nre::worker::NreWorkerClient>*>(NULL));
		ia.register_type(static_cast<DummyWorkflowEngine*>(NULL));
		ia.register_type(static_cast<DaemonFSM*>(NULL));
		ia.register_type(static_cast<GenericDaemon*>(NULL));
		ia.register_type(static_cast<SchedulerImpl*>(NULL));
		ia.register_type(static_cast<SchedulerNRE<sdpa::nre::worker::NreWorkerClient>*>(NULL));
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

void WorkerSerializationTest::testAggregatorSerialization()
{
	std::cout<<std::endl<<"----------------Begin  testAggregatorSerialization----------------"<<std::endl;
	std::string filename = "testSerializeAggregator.txt"; // = boost::archive::tmpdir());filename += "/testfile";
	Aggregator<DummyWorkflowEngine>::ptr_t pAgg = sdpa::daemon::Aggregator<DummyWorkflowEngine>::create("aggregator_0", "127.0.0.1:7001","orchestrator_0"); //, "127.0.0.1:7000");

	pAgg->ptr_scheduler_ = SchedulerImpl::ptr_t(new SchedulerImpl());
	SchedulerImpl* pScheduler = dynamic_cast<SchedulerImpl*>(pAgg->ptr_scheduler_.get());

	JobId id1("_1");
	sdpa::daemon::Job::ptr_t  p1(new JobFSM(id1, "decsription 1"));
	pAgg->ptr_job_man_->addJob(id1, p1);

	JobId id2("_2");
	sdpa::daemon::Job::ptr_t  p2(new JobFSM(id2, "decsription 2"));
	pAgg->ptr_job_man_->addJob(id2, p2);

	JobId id3("_3");
	sdpa::daemon::Job::ptr_t  p3(new JobFSM(id3, "decsription 3"));
	pAgg->ptr_job_man_->addJob(id3, p3);

	JobId id4("_4");
	sdpa::daemon::Job::ptr_t  p4(new JobFSM(id4, "decsription 4"));
	pAgg->ptr_job_man_->addJob(id4, p4);

	JobId id5("_5");
	sdpa::daemon::Job::ptr_t  p5(new JobFSM(id5, "decsription 5"));
	pAgg->ptr_job_man_->addJob(id5, p5);

	int nSchedQSize = 5;
	for(int i=0; i<nSchedQSize; i++)
	{
		std::ostringstream ossJobId;;
		ossJobId<<"Job_"<<i;
		sdpa::job_id_t jobId(ossJobId.str());
		pScheduler->jobs_to_be_scheduled.push(jobId);
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

			const we::preference_t job_pref;
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
		oa.register_type(static_cast<Aggregator<DummyWorkflowEngine>*>(NULL));
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
		Aggregator<DummyWorkflowEngine>::ptr_t pRestoredAgg;
		std::ifstream ifs(filename.c_str());
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<Aggregator<DummyWorkflowEngine>*>(NULL));
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


void WorkerSerializationTest::testOrchestratorSerialization()
{
	std::cout<<std::endl<<"----------------Begin  testOrchestratorSerialization----------------"<<std::endl;
	std::string filename = "testSerializeOrchestrator.txt"; // = boost::archive::tmpdir());filename += "/testfile";
	Orchestrator<DummyWorkflowEngine>::ptr_t pOrch = sdpa::daemon::Orchestrator<DummyWorkflowEngine>::create( "orchestrator_0", "127.0.0.1:6000", "workflows");

	pOrch->ptr_scheduler_ = SchedulerImpl::ptr_t(new SchedulerImpl());
	SchedulerImpl* pScheduler = dynamic_cast<SchedulerImpl*>(pOrch->ptr_scheduler_.get());

	JobId id1("_1");
	sdpa::daemon::Job::ptr_t  p1(new JobFSM(id1, "decsription 1"));
	pOrch->ptr_job_man_->addJob(id1, p1);

	JobId id2("_2");
	sdpa::daemon::Job::ptr_t  p2(new JobFSM(id2, "decsription 2"));
	pOrch->ptr_job_man_->addJob(id2, p2);

	JobId id3("_3");
	sdpa::daemon::Job::ptr_t  p3(new JobFSM(id3, "decsription 3"));
	pOrch->ptr_job_man_->addJob(id3, p3);

	JobId id4("_4");
	sdpa::daemon::Job::ptr_t  p4(new JobFSM(id4, "decsription 4"));
	pOrch->ptr_job_man_->addJob(id4, p4);

	JobId id5("_5");
	sdpa::daemon::Job::ptr_t  p5(new JobFSM(id5, "decsription 5"));
	pOrch->ptr_job_man_->addJob(id5, p5);

	int nSchedQSize = 5;
	for(int i=0; i<nSchedQSize; i++)
	{
		std::ostringstream ossJobId;;
		ossJobId<<"Job_"<<i;
		sdpa::job_id_t jobId(ossJobId.str());
		pScheduler->jobs_to_be_scheduled.push(jobId);
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

			const we::preference_t job_pref;
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
		oa.register_type(static_cast<Orchestrator<DummyWorkflowEngine>*>(NULL));
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
		Orchestrator<DummyWorkflowEngine>::ptr_t pRestoredOrch;
		std::ifstream ifs(filename.c_str());
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<Orchestrator<DummyWorkflowEngine>*>(NULL));
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


void WorkerSerializationTest::testDaemonSerializationWithFSMs()
{
	std::string filename = "testSerializeDaemons.txt"; // = boost::archive::tmpdir());filename += "/testfile";
	GenericDaemon::ptr_t pGenDaemon( new GenericDaemon( sdpa::daemon::ORCHESTRATOR, NULL ));

	pGenDaemon->ptr_scheduler_ = SchedulerImpl::ptr_t(new SchedulerImpl());
	SchedulerImpl* pScheduler = dynamic_cast<SchedulerImpl*>(pGenDaemon->ptr_scheduler_.get());

	// First fill-in the JobManager ...
	JobId id1("_1");
	sdpa::daemon::Job::ptr_t  p1(new JobFSM(id1, "decsription 1"));
	pGenDaemon->ptr_job_man_->addJob(id1, p1);

	JobId id2("_2");
	sdpa::daemon::Job::ptr_t  p2(new JobFSM(id2, "decsription 2"));
	pGenDaemon->ptr_job_man_->addJob(id2, p2);

	JobId id3("_3");
	sdpa::daemon::Job::ptr_t  p3(new JobFSM(id3, "decsription 3"));
	pGenDaemon->ptr_job_man_->addJob(id3, p3);

	JobId id4("_4");
	sdpa::daemon::Job::ptr_t  p4(new JobFSM(id4, "decsription 4"));
	pGenDaemon->ptr_job_man_->addJob(id4, p4);

	JobId id5("_5");
	sdpa::daemon::Job::ptr_t  p5(new JobFSM(id5, "decsription 5"));
	pGenDaemon->ptr_job_man_->addJob(id5, p5);


	/// And then, the scheduler
	int nSchedQSize = 5;
	for(int i=0; i<nSchedQSize; i++)
	{
		std::ostringstream ossJobId;;
		ossJobId<<"Job_"<<i;
		sdpa::job_id_t jobId(ossJobId.str());
		pScheduler->jobs_to_be_scheduled.push(jobId);
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

			const we::preference_t job_pref;
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


void WorkerSerializationTest::testDaemonSerialization()
{
	std::string filename = "testSerializeDaemons.txt"; // = boost::archive::tmpdir());filename += "/testfile";
	GenericDaemon::ptr_t pGenDaemon( new GenericDaemon( sdpa::daemon::ORCHESTRATOR, NULL ));

	pGenDaemon->ptr_scheduler_ = SchedulerImpl::ptr_t(new SchedulerImpl());
	SchedulerImpl* pScheduler = dynamic_cast<SchedulerImpl*>(pGenDaemon->ptr_scheduler_.get());

	// First fill-in the JobManager ...
	JobId id1("_1");
	sdpa::daemon::Job::ptr_t  p1(new sdpa::daemon::JobImpl(id1, "decsription 1"));
	pGenDaemon->ptr_job_man_->addJob(id1, p1);

	JobId id2("_2");
	sdpa::daemon::Job::ptr_t  p2(new sdpa::daemon::JobImpl(id2, "decsription 2"));
	pGenDaemon->ptr_job_man_->addJob(id2, p2);

	JobId id3("_3");
	sdpa::daemon::Job::ptr_t  p3(new sdpa::daemon::JobImpl(id3, "decsription 3"));
	pGenDaemon->ptr_job_man_->addJob(id3, p3);

	JobId id4("_4");
	sdpa::daemon::Job::ptr_t  p4(new sdpa::daemon::JobImpl(id4, "decsription 4"));
	pGenDaemon->ptr_job_man_->addJob(id4, p4);

	JobId id5("_5");
	sdpa::daemon::Job::ptr_t  p5(new sdpa::daemon::JobImpl(id5, "decsription 5"));
	pGenDaemon->ptr_job_man_->addJob(id5, p5);


	/// And then, the scheduler
	int nSchedQSize = 5;
	for(int i=0; i<nSchedQSize; i++)
	{
		std::ostringstream ossJobId;;
		ossJobId<<"Job_"<<i;
		sdpa::job_id_t jobId(ossJobId.str());
		pScheduler->jobs_to_be_scheduled.push(jobId);
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

			const we::preference_t job_pref;
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

void WorkerSerializationTest::testSchedulerSerialization()
{
	std::string filename = "testSerializeScheduler.txt"; // = boost::archive::tmpdir());filename += "/testfile";
	Scheduler::ptr_t pScheduler(new SchedulerImpl());

	int nSchedQSize = 5;
	for(int i=0; i<nSchedQSize; i++)
	{
		std::ostringstream ossJobId;;
		ossJobId<<"Job_"<<i;
		sdpa::job_id_t jobId(ossJobId.str());
		dynamic_cast<SchedulerImpl*>(pScheduler.get())->jobs_to_be_scheduled.push(jobId);
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

			const we::preference_t job_pref;
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


void WorkerSerializationTest::testWorkerSerialization()
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

void WorkerSerializationTest::testSynchQueueSerialization()
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

void WorkerSerializationTest::testBackupRecoverOrch()
{
	std::cout<<std::endl<<"----------------Begin  testBackupRecoverOrch----------------"<<std::endl;
	std::string filename = "testBackupRecover.txt"; // = boost::archive::tmpdir());filename += "/testfile";

	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	cav.push_back("--orchestrator=orchestrator_0");
	cav.push_back("--network.location=orchestrator_0:127.0.0.1:7000");
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config );
	ptrCli->configure_network( config );

	std::string strWorkflow = read_workflow("workflows/stresstest.pnet");

	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::ptr_t ptrOrch = sdpa::daemon::Orchestrator<DummyWorkflowEngine>::create("orchestrator_0", "127.0.0.1:7000", "workflows");
	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::start(ptrOrch);

	// submit a number of jobs
	for(int k=0; k<5; k++ )
		sdpa::job_id_t job_id_user = ptrCli->submitJob(strWorkflow);

	ptrOrch->print();
	ptrOrch->backup(filename);
	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::shutdown(ptrOrch);
	sleep(1);

	ptrCli->shutdown_network();
	seda::StageRegistry::instance().clear();

	// now try to recover the system
	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::ptr_t ptrRecOrch = sdpa::daemon::Orchestrator<DummyWorkflowEngine>::create("orchestrator_0", "127.0.0.1:7000", "workflows" );
	ptrRecOrch->recover(filename);
	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::start(ptrRecOrch);

	sleep(1);

	sdpa::daemon::Orchestrator<DummyWorkflowEngine>::shutdown(ptrRecOrch);
	sleep(1);

	std::cout<<std::endl<<"----------------End  testBackupRecoverOrch----------------"<<std::endl;
}
