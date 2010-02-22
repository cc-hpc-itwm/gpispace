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
#include <sdpa/daemon/aggregator/Aggregator.hpp>
#include <sdpa/daemon/nre/NRE.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <sdpa/daemon/nre/SchedulerNRE.hpp>

using namespace sdpa::tests;
using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION( sdpa::tests::WorkerSerializationTest );

WorkerSerializationTest::WorkerSerializationTest() {
}

WorkerSerializationTest::~WorkerSerializationTest() {
}

void WorkerSerializationTest::setUp() {
}

void WorkerSerializationTest::tearDown() {
}

void WorkerSerializationTest::testNRESerialization()
{
	/*std::cout<<std::endl<<"----------------Begin  testNRESerialization----------------"<<std::endl;
	std::string filename = "testSerializeNRE.txt"; // = boost::archive::tmpdir());filename += "/testfile";
	sdpa::daemon::NRE::ptr_t ptrNRE_0 = sdpa::daemon::NRE::create("NRE_0",  "127.0.0.1:7002","aggregator_0", "127.0.0.1:7001", "127.0.0.1:8000" );

	ptrNRE_0->ptr_scheduler_ = sdpa::daemon::SchedulerNRE::ptr_t(new sdpa::daemon::SchedulerNRE());
	 sdpa::daemon::SchedulerNRE* pScheduler = dynamic_cast< sdpa::daemon::SchedulerNRE*>(ptrNRE_0->ptr_scheduler_.get());

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

	int nWorkers=0; // no workers

	try
	{
		std::cout<<"----------------The NRE's content before backup is:----------------"<<std::endl;
		ptrNRE_0->print();

		std::ofstream ofs(filename.c_str());
		boost::archive::text_oarchive oa(ofs);
		oa.register_type(static_cast<DaemonFSM*>(NULL));
		oa.register_type(static_cast<GenericDaemon*>(NULL));
		oa.register_type(static_cast<SchedulerImpl*>(NULL));
		oa.register_type(static_cast<SchedulerNRE*>(NULL));
		oa.register_type(static_cast<JobFSM*>(NULL));
		oa.register_type(static_cast< sdpa::daemon::NRE*>(NULL));
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
		NRE::ptr_t ptrRestoredNRE_0;
		std::ifstream ifs(filename.c_str());
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<DaemonFSM*>(NULL));
		ia.register_type(static_cast<GenericDaemon*>(NULL));
		ia.register_type(static_cast<SchedulerImpl*>(NULL));
		ia.register_type(static_cast<SchedulerNRE*>(NULL));
		ia.register_type(static_cast<JobFSM*>(NULL));
		ia.register_type(static_cast< sdpa::daemon::NRE*>(NULL));
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
	*/
}

void WorkerSerializationTest::testAggregatorSerialization()
{
	std::cout<<std::endl<<"----------------Begin  testAggregatorSerialization----------------"<<std::endl;
	std::string filename = "testSerializeAggregator.txt"; // = boost::archive::tmpdir());filename += "/testfile";
	Aggregator::ptr_t pAgg = sdpa::daemon::Aggregator::create("aggregator_0", "127.0.0.1:7001","orchestrator_0", "127.0.0.1:7000");

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
		Worker::ptr_t pWorker( new Worker(ossWorkerId.str()) );
		for( int l=0; l<3; l++)
		{
			std::ostringstream ossJobId;
			ossJobId<<"Job_"<<k*nWorkers + l + nSchedQSize;
			sdpa::job_id_t jobId(ossJobId.str());
			pWorker->dispatch(jobId);
			if(l>=1)
			{
				sdpa::job_id_t jobToSubmit = pWorker->get_next_job("");
				if(l>=2)
					pWorker->acknowledge(jobToSubmit);
			}
		}

		pScheduler->addWorker(pWorker);
	}

	try
	{
		std::cout<<"----------------The Aggregator's content before backup is:----------------"<<std::endl;
		pAgg->print();

		std::ofstream ofs(filename.c_str());
		boost::archive::text_oarchive oa(ofs);
		oa.register_type(static_cast<Aggregator*>(NULL));
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

	std::cout<<"----------------Try now to restore the Aggregator:----------------"<<std::endl;
	try
	{
		Aggregator::ptr_t pRestoredAgg;
		std::ifstream ifs(filename.c_str());
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<Aggregator*>(NULL));
		ia.register_type(static_cast<DaemonFSM*>(NULL));
		ia.register_type(static_cast<GenericDaemon*>(NULL));
		ia.register_type(static_cast<SchedulerImpl*>(NULL));
		ia.register_type(static_cast<JobFSM*>(NULL));
		ia >> pRestoredAgg;

		std::cout<<std::endl<<"----------------The restored content of the Aggregator is:----------------"<<std::endl;
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
	Orchestrator::ptr_t pOrch = sdpa::daemon::Orchestrator::create( "orchestrator_0", "127.0.0.1:6000", "workflows");

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
		Worker::ptr_t pWorker( new Worker(ossWorkerId.str()) );
		for( int l=0; l<3; l++)
		{
			std::ostringstream ossJobId;
			ossJobId<<"Job_"<<k*nWorkers + l + nSchedQSize;
			sdpa::job_id_t jobId(ossJobId.str());
			pWorker->dispatch(jobId);
			if(l>=1)
			{
				sdpa::job_id_t jobToSubmit = pWorker->get_next_job("");
				if(l>=2)
					pWorker->acknowledge(jobToSubmit);
			}
		}

		pScheduler->addWorker(pWorker);
	}

	try
	{
		std::cout<<"----------------The Orchestrator's content before backup is:----------------"<<std::endl;
		pOrch->print();

		std::ofstream ofs(filename.c_str());
		boost::archive::text_oarchive oa(ofs);
		oa.register_type(static_cast<Orchestrator*>(NULL));
		oa.register_type(static_cast<DaemonFSM*>(NULL));
		oa.register_type(static_cast<GenericDaemon*>(NULL));
		oa.register_type(static_cast<SchedulerImpl*>(NULL));
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
		ia.register_type(static_cast<DaemonFSM*>(NULL));
		ia.register_type(static_cast<GenericDaemon*>(NULL));
		ia.register_type(static_cast<SchedulerImpl*>(NULL));
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
		Worker::ptr_t pWorker( new Worker(ossWorkerId.str()) );
		for( int l=0; l<3; l++)
		{
			std::ostringstream ossJobId;
			ossJobId<<"Job_"<<k*nWorkers + l + nSchedQSize;
			sdpa::job_id_t jobId(ossJobId.str());
			pWorker->dispatch(jobId);
			if(l>=1)
			{
				sdpa::job_id_t jobToSubmit = pWorker->get_next_job("");
				if(l>=2)
					pWorker->acknowledge(jobToSubmit);
			}
		}

		pScheduler->addWorker(pWorker);
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
		Worker::ptr_t pWorker( new Worker(ossWorkerId.str()) );
		for( int l=0; l<3; l++)
		{
			std::ostringstream ossJobId;
			ossJobId<<"Job_"<<k*nWorkers + l + nSchedQSize;
			sdpa::job_id_t jobId(ossJobId.str());
			pWorker->dispatch(jobId);
			if(l>=1)
			{
				sdpa::job_id_t jobToSubmit = pWorker->get_next_job("");
				if(l>=2)
					pWorker->acknowledge(jobToSubmit);
			}
		}

		pScheduler->addWorker(pWorker);
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
		Worker::ptr_t pWorker( new Worker(ossWorkerId.str()) );
		for( int l=0; l<3; l++)
		{
			std::ostringstream ossJobId;
			ossJobId<<"Job_"<<k*nWorkers + l + nSchedQSize;
			sdpa::job_id_t jobId(ossJobId.str());
			pWorker->dispatch(jobId);
			if(l>=1)
			{
				sdpa::job_id_t jobToSubmit = pWorker->get_next_job("");
				if(l>=2)
					pWorker->acknowledge(jobToSubmit);
			}
		}

		dynamic_cast<SchedulerImpl*>(pScheduler.get())->addWorker(pWorker);
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

