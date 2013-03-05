#define BOOST_TEST_MODULE TestAgentsSerialization
#include <sdpa/daemon/JobFSM.hpp>

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
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

#include <sdpa/engine/DummyWorkflowEngine.hpp>

#include <sdpa/client/ClientApi.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/Strategy.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>
#include <boost/thread.hpp>
#include <fstream>
#include <sstream>

using namespace sdpa::tests;
using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("12100"); return s; }


struct MyFixture
{
	MyFixture()
			: m_nITER(1)
			, m_sleep_interval(1000000)
	{ //initialize and start the finite state machine

		//FHGLOG_SETUP();

		LOG(DEBUG, "Fixture's constructor called ...");

		m_pool = new fhg::com::io_service_pool(1);
		m_kvsd = new fhg::com::kvs::server::kvsd ("/tmp/notthere");
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

		m_serv->stop ();
		m_pool->stop ();
		m_thrd->join ();

		delete m_thrd;
		delete m_serv;
		delete m_kvsd;
		delete m_pool;

		seda::StageRegistry::instance().stopAll();
		seda::StageRegistry::instance().clear();
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

	fhg::com::io_service_pool *m_pool;
	fhg::com::kvs::server::kvsd *m_kvsd;
	fhg::com::tcp_server *m_serv;
	boost::thread *m_thrd;
};

BOOST_AUTO_TEST_SUITE( test_SerializeAgents );

BOOST_AUTO_TEST_CASE(testBackupRecoverJobManager)
{
	std::cout<<"Testing the JobManager serialization ..."<<std::endl;
	JobManager* pJobMan = new JobManager();

	JobId id1("_1");
	sdpa::daemon::Job::ptr_t  p1(new JobFSM(id1, "decsription 1"));
	pJobMan->addJob(id1, p1);

	JobId id2("_2");
	sdpa::daemon::Job::ptr_t  p2(new JobFSM(id2, "decsription 2"));
	pJobMan->addJob(id2, p2);

	JobId id3("_3");
	sdpa::daemon::Job::ptr_t  p3(new JobFSM(id3, "decsription 3"));
	pJobMan->addJob(id3, p3);

	JobId id4("_4");
	sdpa::daemon::Job::ptr_t  p4(new JobFSM(id4, "decsription 4"));
	pJobMan->addJob(id4, p4);

	JobId id5("_5");
	sdpa::daemon::Job::ptr_t  p5(new JobFSM(id5, "decsription 5"));
	pJobMan->addJob(id5, p5);

	pJobMan->print();
	std::string filename("JobManagerPersistFile.bkp");

	LOG(INFO, "Backup theJobManager to file "<<filename);
	//backup(name()+".bkp");
	std::ofstream ofs(filename.c_str());
	boost::archive::text_oarchive oa(ofs);
	oa.register_type(static_cast<sdpa::daemon::JobManager*>(NULL));
	oa.register_type(static_cast<sdpa::daemon::JobImpl*>(NULL));
	oa.register_type(static_cast<JobFSM*>(NULL));
	oa << pJobMan;

	ofs.close();

	std::cout<<"Restore the JobManager from file "<<filename<<std::endl;
	// open the archive
	std::ifstream ifs(filename.c_str());
	boost::archive::text_iarchive ia(ifs);
	ia.register_type(static_cast<sdpa::daemon::JobManager*>(NULL));
	ia.register_type(static_cast<sdpa::daemon::JobImpl*>(NULL));
	ia.register_type(static_cast<JobFSM*>(NULL));
	// restore the schedule from the archive

	JobManager* pJobManRestored;
	ia >> pJobManRestored;
	ifs.close();

	LOG(INFO, "JobManager after recovering" );
	pJobManRestored->print();

	std::ofstream ofs2("RecoveredJobManager.txt");
	boost::archive::text_oarchive oa2(ofs2);
	oa2.register_type(static_cast<sdpa::daemon::JobManager*>(NULL));
	oa2.register_type(static_cast<sdpa::daemon::JobImpl*>(NULL));
	oa2.register_type(static_cast<JobFSM*>(NULL));
	oa2<< pJobManRestored;
	ofs2.close();
}

BOOST_AUTO_TEST_CASE( testSerializeMapJobRawPtr )
{
	std::cout<<"Testing the serialization of a map of normal job pointers ..."<<std::endl;
    std::string filename = "testSerializeMapJobPtr.txt"; // = boost::archive::tmpdir());filename += "/testfile";

    std::map<std::string, sdpa::daemon::Job*> mapPtrsOut;

    mapPtrsOut["1"] = new sdpa::daemon::JobImpl("first", "decsription 1");
    mapPtrsOut["2"] = new sdpa::daemon::JobImpl("second", "decsription 2");
    mapPtrsOut["3"] = new sdpa::daemon::JobImpl("third", "decsription 3");
    mapPtrsOut["4"] = new sdpa::daemon::JobImpl("fourth", "decsription 4");
    mapPtrsOut["5"] = new sdpa::daemon::JobImpl("fifth", "decsription 5");

    LOG(INFO, "Before ......................"<<std::endl);

	//std::cout<<"The restored value of the pointer is "<<p2->str()<<endl;
	for(  std::map<std::string, sdpa::daemon::Job*>::iterator iter = mapPtrsOut.begin(); iter !=  mapPtrsOut.end(); iter++  )
		std::cout<<"mapPtrsOut["<<iter->first<<"] = "<<iter->second->print_info()<<std::endl;

    // serialize it
	{
	   std::ofstream ofs(filename.c_str());
	   boost::archive::text_oarchive oa(ofs);
	   oa.register_type(static_cast<sdpa::daemon::JobImpl *>(NULL));
	   oa << BOOST_SERIALIZATION_NVP(mapPtrsOut);
	}

	for(  std::map<std::string, sdpa::daemon::Job*>::iterator it = mapPtrsOut.begin(); it !=  mapPtrsOut.end(); it++  )
		if(it->second)
			delete it->second;

	// restore state to one equivalent to the original
	// creating a new type sdpa::daemon::Job object
	std::map<std::string, sdpa::daemon::Job*> mapPtrsIn;
	{
	   // open the archive
	   std::ifstream ifs(filename.c_str());
	   boost::archive::text_iarchive ia(ifs);
	   ia.register_type(static_cast<sdpa::daemon::JobImpl *>(NULL));
	   ia >> BOOST_SERIALIZATION_NVP(mapPtrsIn);
	}

	LOG(INFO, "After ......................"<<std::endl);
	//std::cout<<"The restored value of the pointer is "<<p2->str()<<endl;
	for(  std::map<std::string, sdpa::daemon::Job*>::iterator iter = mapPtrsIn.begin(); iter !=  mapPtrsIn.end(); iter++  )
		std::cout<<"mapPtrIn["<<iter->first<<"] = "<<iter->second->print_info()<<std::endl;

	for(  std::map<std::string, sdpa::daemon::Job*>::iterator iter = mapPtrsIn.begin(); iter !=  mapPtrsIn.end(); iter++  )
		if(iter->second)
			delete iter->second;
}

BOOST_AUTO_TEST_CASE( testSerializeMapJobShPtr )
{
	std::cout<<"Testing the serialization of a map of job shared pointers ..."<<std::endl;
    std::string filename = "testSerializeMapJobPtr.txt"; // = boost::archive::tmpdir());filename += "/testfile";

    sdpa::daemon::JobManager::job_map_t mapPtrsOut;
    typedef  sdpa::daemon::JobManager::job_map_t::value_type pair;

    JobImpl::ptr_t p1( new sdpa::daemon::JobImpl("1", "decsription 1"));
    mapPtrsOut.insert( pair("1", p1) );

    JobImpl::ptr_t p2( new sdpa::daemon::JobImpl("2", "decsription 2"));
    mapPtrsOut.insert( pair("2", p2) );

    JobImpl::ptr_t p3( new sdpa::daemon::JobImpl("3", "decsription 3"));
    mapPtrsOut.insert( pair("3", p3) );

    JobImpl::ptr_t p4( new sdpa::daemon::JobImpl("4", "decsription 4"));
    mapPtrsOut.insert( pair("4", p4) );

    JobImpl::ptr_t p5( new sdpa::daemon::JobImpl("5", "decsription 5"));
    mapPtrsOut.insert( pair("5", p5) );

    LOG(INFO, "Before ......................"<<std::endl);

	//std::cout<<"The restored value of the pointer is "<<p2->str()<<endl;
	for(  sdpa::daemon::JobManager::job_map_t::iterator iter = mapPtrsOut.begin(); iter !=  mapPtrsOut.end(); iter++  )
		std::cout<<"mapPtrsOut["<<iter->first<<"] = "<<iter->second->print_info()<<std::endl;

    // serialize it
	{
	   std::ofstream ofs(filename.c_str());
	   boost::archive::text_oarchive oa(ofs);
	   oa.register_type(static_cast<sdpa::daemon::JobImpl *>(NULL));
	   oa << BOOST_SERIALIZATION_NVP(mapPtrsOut);
	}

	// restore state to one equivalent to the original
	// creating a new type sdpa::daemon::Job object
	sdpa::daemon::JobManager::job_map_t mapPtrsIn;
	{
	   // open the archive
	   std::ifstream ifs(filename.c_str());
	   boost::archive::text_iarchive ia(ifs);
	   ia.register_type(static_cast<sdpa::daemon::JobImpl *>(NULL));
	   ia >> BOOST_SERIALIZATION_NVP(mapPtrsIn);
	}

	LOG(INFO, "After ......................"<<std::endl);
	//std::cout<<"The restored value of the pointer is "<<p2->str()<<endl;
	for(  sdpa::daemon::JobManager::job_map_t::iterator iter = mapPtrsIn.begin(); iter !=  mapPtrsIn.end(); iter++  )
		std::cout<<"mapPtrIn["<<iter->first<<"] = "<<iter->second->print_info()<<std::endl;
}

BOOST_AUTO_TEST_CASE(testOrchFileSerialization)
{
	std::cout<<std::endl<<"----------------Begin  testOrchestratorSerialization----------------"<<std::endl;
	std::string filename = "testSerializeOrchestrator.txt"; // = boost::archive::tmpdir());filename += "/testfile";
	Orchestrator::ptr_t pOrchBkp = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create( "orchestrator_0", "127.0.0.1:6000");

	pOrchBkp->setScheduler(new SchedulerImpl());
	SchedulerImpl* pScheduler = dynamic_cast<SchedulerImpl*>(pOrchBkp->scheduler().get());
	JobManager::ptr_t pJobMan(pOrchBkp->jobManager());

	JobId id1("_1");
	sdpa::daemon::Job::ptr_t  p1(new JobFSM(id1, "decsription 1"));
	pJobMan->addJob(id1, p1);

	JobId id2("_2");
	sdpa::daemon::Job::ptr_t  p2(new JobFSM(id2, "decsription 2"));
	pJobMan->addJob(id2, p2);

	JobId id3("_3");
	sdpa::daemon::Job::ptr_t  p3(new JobFSM(id3, "decsription 3"));
	pJobMan->addJob(id3, p3);

	JobId id4("_4");
	sdpa::daemon::Job::ptr_t  p4(new JobFSM(id4, "decsription 4"));
	pJobMan->addJob(id4, p4);

	JobId id5("_5");
	sdpa::daemon::Job::ptr_t  p5(new JobFSM(id5, "decsription 5"));
	pJobMan->addJob(id5, p5);

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

			pScheduler->schedule_to(jobId, k);
			if(l>=1)
			{
				sdpa::job_id_t jobToSubmit = pScheduler->getNextJob(workerId, "");
				if(l>=2)
					pScheduler->acknowledgeJob(workerId, jobToSubmit);
			}
		}
	}

	std::cout<<"----------------The Orchestrator's content before backup is:----------------"<<std::endl;
	std::ofstream os("orchestrator.bkp");
	pOrchBkp->backup(os);
	os.close();
	pOrchBkp->print();
	seda::StageRegistry::instance().remove("orchestrator_0");

    Orchestrator::ptr_t pOrchRec = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create( "orchestrator_1", "127.0.0.1:6000");

    std::ifstream is("orchestrator.bkp");
	pOrchRec->recover(is);
	is.close();
	std::cout<<"----------------The Orchestrator's content after recovering is:----------------"<<std::endl;
	pOrchRec->print();
	seda::StageRegistry::instance().remove("orchestrator_1");
}

BOOST_AUTO_TEST_CASE(testOrchStringSerialization)
{
	std::cout<<std::endl<<"----------------Begin  testOrchestratorSerialization----------------"<<std::endl;
	std::string filename = "testSerializeOrchestrator.txt"; // = boost::archive::tmpdir());filename += "/testfile";
	Orchestrator::ptr_t pOrchBkp = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create( "orchestrator_0", "127.0.0.1:6000");

	pOrchBkp->setScheduler(new SchedulerImpl());
	SchedulerImpl* pScheduler = dynamic_cast<SchedulerImpl*>(pOrchBkp->scheduler().get());
	JobManager::ptr_t pJobMan(pOrchBkp->jobManager());

	JobId id1("_1");
	sdpa::daemon::Job::ptr_t  p1(new JobFSM(id1, "decsription 1"));
	pJobMan->addJob(id1, p1);

	JobId id2("_2");
	sdpa::daemon::Job::ptr_t  p2(new JobFSM(id2, "decsription 2"));
	pJobMan->addJob(id2, p2);

	JobId id3("_3");
	sdpa::daemon::Job::ptr_t  p3(new JobFSM(id3, "decsription 3"));
	pJobMan->addJob(id3, p3);

	JobId id4("_4");
	sdpa::daemon::Job::ptr_t  p4(new JobFSM(id4, "decsription 4"));
	pJobMan->addJob(id4, p4);

	JobId id5("_5");
	sdpa::daemon::Job::ptr_t  p5(new JobFSM(id5, "decsription 5"));
	pJobMan->addJob(id5, p5);

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

			pScheduler->schedule_to(jobId, k);
			if(l>=1)
			{
				sdpa::job_id_t jobToSubmit = pScheduler->getNextJob(workerId, "");
				if(l>=2)
					pScheduler->acknowledgeJob(workerId, jobToSubmit);
			}
		}
	}

	std::cout<<"----------------The Orchestrator's content before backup is:----------------"<<std::endl;
	std::stringstream ss;
	pOrchBkp->backup(ss);
	pOrchBkp->print();
	seda::StageRegistry::instance().remove("orchestrator_0");

	Orchestrator::ptr_t pOrchRec = sdpa::daemon::OrchestratorFactory<DummyWorkflowEngine>::create( "orchestrator_1", "127.0.0.1:6000");

	//std::istringstream iss(oss.str());
	pOrchRec->recover(ss);
	std::cout<<"----------------The Orchestrator's content after recovering is:----------------"<<std::endl;
	pOrchRec->print();
	seda::StageRegistry::instance().remove("orchestrator_1");
}

BOOST_AUTO_TEST_SUITE_END()
