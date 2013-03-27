/*
 * =====================================================================================
 *
 *       Filename:  test_SerializeJobPtr.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================*/
#define BOOST_TEST_MODULE TestSerializeJobPtrs
#include <sdpa/daemon/JobFSM.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <cstddef>
#include <fstream>
#include <string>
#include <cstdio>
#include <boost/config.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>
#include "boost/serialization/map.hpp"
#include <sdpa/daemon/JobManager.hpp>

#include <boost/serialization/export.hpp>
#include <sdpa/JobId.hpp>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"

using namespace sdpa;

using namespace std;
//using namespace sdpa::tests;

namespace sdpa {
        namespace unit_test {

        class Job
        {
        public:
          Job(string /* arg */) { std::cout<<"Called the constructor ..."<<std::endl;}
                 Job(){std::cout<<"Called the constructor ..."<<std::endl;}
                 virtual ~Job(){std::cout<<"Called the destructor ..."<<std::endl;}
                 virtual string print_info() = 0;

        private:
                friend class boost::serialization::access;

                template<class Archive>
                void serialize(Archive& /*ar*/, const unsigned int /* file version */){}
        };

        class JobImpl : public Job
        {
        public:
        	JobImpl(const sdpa::job_id_t id = sdpa::JobId(""),
        			const sdpa::job_desc_t /* desc */ = "",
        			const sdpa::daemon::IAgent* /* pHandler */ = NULL,
        			const sdpa::job_id_t &/*parent*/ = sdpa::job_id_t::invalid_job_id()) : Job(id.str())
                {
					parent_ = "aaaaaaaaaaaaa";
					worker_id_ = "bbbbbbbbbbbbb";
					desc_ = "ccccccccccccc";
                }

                virtual std::string print_info()
                {
					ostringstream os;
					os<<id_<<std::endl;
					os<<desc_<<std::endl;
					os<<parent_<<std::endl;
					os<<worker_id_<<std::endl;
					return os.str();
                }

                virtual ~JobImpl() {};

        private:
                friend class boost::serialization::access;

                template<class Archive>
                void serialize(Archive & ar, const unsigned int /* file_version */)
                {
					ar & boost::serialization::base_object<Job>(*this);
					ar & id_;
					ar & desc_;
					ar & parent_;
					ar & worker_id_;
                }

                sdpa::job_id_t id_;
                sdpa::job_desc_t desc_;
                sdpa::job_id_t parent_;
                sdpa::worker_id_t worker_id_;
        };
}}


struct MyFixture
{
	MyFixture() :SDPA_INIT_LOGGER("sdpa.tests. TestSerializeJobPtr")
	{
		FHGLOG_SETUP();
	}

	~MyFixture(){}
	SDPA_DECLARE_LOGGER();
};

BOOST_FIXTURE_TEST_SUITE( test_SerializeJobPtr, MyFixture )

BOOST_SERIALIZATION_ASSUME_ABSTRACT( sdpa::unit_test::Job )
BOOST_SERIALIZATION_ASSUME_ABSTRACT( sdpa::daemon::Job )

BOOST_AUTO_TEST_CASE(testSerializeJobPtr )
{
	std::cout<<"Testing the serialization of a normal job pointer ..."<<std::endl;
	std::string filename = "testSerializeJobPtr.txt"; // = boost::archive::tmpdir());filename += "/testfile";

	sdpa::unit_test::Job* p1 = new sdpa::unit_test::JobImpl("1234", "a certain job description");

	// serialize it
	{
	   std::ofstream ofs(filename.c_str());
	   boost::archive::text_oarchive oa(ofs);
	   oa.register_type(static_cast<sdpa::unit_test::JobImpl*>(NULL));
	   oa << p1;
	}

	// restore state to one equivalent to the original
	// creating a new type Job object
	sdpa::unit_test::Job* p2 = NULL;
	{
	   // open the archive
	   std::ifstream ifs(filename.c_str());
	   boost::archive::text_iarchive ia(ifs);
	   ia.register_type(static_cast<sdpa::unit_test::JobImpl*>(NULL));
	   // restore the schedule from the archive
	   ia>>p2;
	}

	std::cout<<"The restored values from the archive follow:\n"<<p2->print_info()<<std::endl;
	delete p1;
	delete p2;
}

BOOST_AUTO_TEST_CASE( testSerializeSdpaJobPtr )
{
	std::cout<<"Testing the serialization of a shared job pointer ..."<<std::endl;
	std::string filename = "testSerializeSdpaJobPtr.txt"; // = boost::archive::tmpdir());filename += "/testfile";

	sdpa::daemon::Job* p1 = new sdpa::daemon::JobImpl("1234", "a certain job description");

	// serialize it
	{
	   std::ofstream ofs(filename.c_str());
	   boost::archive::text_oarchive oa(ofs);
	   oa.register_type(static_cast<sdpa::daemon::JobImpl*>(NULL));
	   oa << p1;
	}

	// restore state to one equivalent to the original
	// creating a new type Job object
	sdpa::daemon::Job* p2 = NULL;
	{
	   // open the archive
	   std::ifstream ifs(filename.c_str());
	   boost::archive::text_iarchive ia(ifs);
	   ia.register_type(static_cast<sdpa::daemon::JobImpl*>(NULL));
	   // restore the schedule from the archive
	   ia>>p2;
	}

	std::cout<<"The restored values from the archive follow:\n"<<p2->print_info()<<endl;
	delete p1;
	delete p2;
}

BOOST_AUTO_TEST_CASE( testSerializeMapJobPtr )
{
	std::cout<<"Testing the serialization of a map of normal job pointers ..."<<std::endl;
    std::string filename = "testSerializeMapJobPtr.txt"; // = boost::archive::tmpdir());filename += "/testfile";

    std::map<std::string, sdpa::daemon::Job*> mapPtrsOut;

    mapPtrsOut["1"] = new sdpa::daemon::JobImpl("first", "decsription 1");
    mapPtrsOut["2"] = new sdpa::daemon::JobImpl("second", "decsription 2");
    mapPtrsOut["3"] = new sdpa::daemon::JobImpl("third", "decsription 3");
    mapPtrsOut["4"] = new sdpa::daemon::JobImpl("fourth", "decsription 4");
    mapPtrsOut["5"] = new sdpa::daemon::JobImpl("fifth", "decsription 5");

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
	   // restore the schedule from the archive
	   ia >> BOOST_SERIALIZATION_NVP(mapPtrsIn);
	}

	//std::cout<<"The restored value of the pointer is "<<p2->str()<<endl;
	for(  std::map<std::string, sdpa::daemon::Job*>::iterator iter = mapPtrsIn.begin(); iter !=  mapPtrsIn.end(); iter++  )
			std::cout<<"mapPtrIn["<<iter->first<<"] = "<<iter->second->print_info()<<std::endl;

	for(  std::map<std::string, sdpa::daemon::Job*>::iterator iter = mapPtrsIn.begin(); iter !=  mapPtrsIn.end(); iter++  )
			if(iter->second)
					delete iter->second;
}

BOOST_AUTO_TEST_CASE(testSerializeSdpaJobSharedPtr )
{
	std::cout<<"Testing the serialization of a job shared pointer ..."<<std::endl;
	std::string filename = "testSerializeSdpaJobSharedPtr.txt"; // = boost::archive::tmpdir());filename += "/testfile";

	sdpa::daemon::Job::ptr_t p1(new sdpa::daemon::JobImpl("1234", "owned by a job shared pointer ..."));

	// serialize it
	{
	   std::ofstream ofs(filename.c_str());
	   boost::archive::text_oarchive oa(ofs);
	   oa.register_type(static_cast<sdpa::daemon::JobImpl*>(NULL));
	   oa << p1;
	}

	// restore state to one equivalent to the original
	// creating a new type Job object
	sdpa::daemon::Job::ptr_t p2;
	{
	   // open the archive
	   std::ifstream ifs(filename.c_str());
	   boost::archive::text_iarchive ia(ifs);
	   ia.register_type(static_cast<sdpa::daemon::JobImpl*>(NULL));
	   // restore the schedule from the archive
	   ia>>p2;
	}

	std::cout<<"The restored values from the archive follow:\n\n"<<p2->print_info()<<endl;
}

BOOST_AUTO_TEST_CASE( testSerializeMapSdpaJobSharedPtr )
{
	std::cout<<"Testing the serialization of a map of job shared pointers ..."<<std::endl;
	std::string filename = "testSerializeMapSdpaJobSharedPtr.txt"; // = boost::archive::tmpdir());filename += "/testfile";

	typedef std::map<sdpa::job_id_t, sdpa::daemon::Job::ptr_t> map_shptr_jobs_t;
	//typedef std::map<std::string, sdpa::daemon::Job::ptr_t> map_shptr_jobs_t;

	map_shptr_jobs_t mapPtrsOut;

	JobId id1("1");
	sdpa::daemon::Job::ptr_t  p1(new sdpa::daemon::JobImpl(id1, "decsription 1"));
	mapPtrsOut[id1] = p1;

	JobId id2("2");
	sdpa::daemon::Job::ptr_t  p2(new sdpa::daemon::JobImpl(id2, "decsription 2"));
	mapPtrsOut[id2] = p2;

	JobId id3("3");
	sdpa::daemon::Job::ptr_t  p3(new sdpa::daemon::JobImpl(id3, "decsription 3"));
	mapPtrsOut[id3] = p3;

	JobId id4("4");
	sdpa::daemon::Job::ptr_t  p4(new sdpa::daemon::JobImpl(id4, "decsription 4"));
	mapPtrsOut[id4] = p4;

	JobId id5("5");
	sdpa::daemon::Job::ptr_t  p5(new sdpa::daemon::JobImpl(id5, "decsription 5"));
	mapPtrsOut[id5] = p5;

	// serialize it
	{
	   std::ofstream ofs(filename.c_str());
	   boost::archive::text_oarchive oa(ofs);
	   oa.register_type(static_cast<sdpa::daemon::JobImpl *>(NULL));
 //! \note Warning: Boost: Storing non-const object with object
 //! tracking not being 'never'.
	   oa << (map_shptr_jobs_t const&) mapPtrsOut;
	}


	// restore state to one equivalent to the original
	// creating a new type sdpa::daemon::Job object
	map_shptr_jobs_t mapPtrsIn;
	{
	   // open the archive
	   std::ifstream ifs(filename.c_str());
	   boost::archive::text_iarchive ia(ifs);
	   ia.register_type(static_cast<sdpa::daemon::JobImpl *>(NULL));
	   // restore the schedule from the archive
	   ia >> mapPtrsIn;
	}


	//std::cout<<"The restored value of the pointer is "<<p2->str()<<endl;
	for(  map_shptr_jobs_t::iterator iter = mapPtrsIn.begin(); iter !=  mapPtrsIn.end(); iter++  )
			std::cout<<"mapPtrIn["<<iter->first<<"] = "<<iter->second->print_info()<<std::endl;
}

BOOST_AUTO_TEST_CASE( testSerializeJobFSMShPtr )
{
	std::cout<<"Testing the serialization of a job state chart ..."<<std::endl;
	std::string filename = "testSerializeJobFSMShPtr.txt"; // = boost::archive::tmpdir());filename += "/testfile";

	JobFSM::Ptr p1(new JobFSM("1234", "owned by a job shared pointer ..."));

	// serialize it
	{
	   std::ofstream ofs(filename.c_str());
	   boost::archive::text_oarchive oa(ofs);
	   oa.register_type(static_cast<JobFSM*>(NULL));
	   oa << p1;
	}

	// restore state to one equivalent to the original
	// creating a new type Job object
	JobFSM::Ptr p2;
	{
	   // open the archive
	   std::ifstream ifs(filename.c_str());
	   boost::archive::text_iarchive ia(ifs);
	   ia.register_type(static_cast<JobFSM*>(NULL));
	   // restore the schedule from the archive
	   ia>>p2;
	}

	LOG(INFO, "The restored values from the archive follow:\n\n"<<p2->print_info());
}

BOOST_SERIALIZATION_SHARED_PTR(sdpa::daemon::JobManager)

BOOST_AUTO_TEST_CASE( testSerializeJobManager )
{
	std::string filename = "JobManager.bak";

	sdpa::daemon::JobManager::ptr_t pJobMan1(new sdpa::daemon::JobManager());

	JobId id1("1");
	sdpa::daemon::Job::ptr_t p1(new JobFSM(id1, "decsription 1"));
	pJobMan1->addJob(id1, p1);

	JobId id2("2");
	sdpa::daemon::Job::ptr_t p2(new JobFSM(id2, "decsription 2"));
	pJobMan1->addJob(id2, p2);

	JobId id3("3");
	sdpa::daemon::Job::ptr_t p3(new JobFSM(id3, "decsription 3"));
	pJobMan1->addJob(id3, p3);

	JobId id4("4");
	sdpa::daemon::Job::ptr_t p4(new JobFSM(id4, "decsription 4"));
	pJobMan1->addJob(id4, p4);

	JobId id5("5");
	sdpa::daemon::Job::ptr_t p5(new JobFSM(id5, "decsription 5"));
	pJobMan1->addJob(id5, p5);

	LOG(DEBUG, pJobMan1->print());

	LOG(INFO, "Serialize the JobManager ...");

	try {
		std::ofstream ofs(filename.c_str());
		boost::archive::text_oarchive oa(ofs);
		oa.register_type(static_cast<sdpa::daemon::JobManager*>(NULL));
		oa.register_type(static_cast<JobFSM*>(NULL));
		oa << pJobMan1;
	}
	catch(std::exception& ex) {
		LOG(ERROR, "Exception occurred during serialization :"<<ex.what());
	}

	LOG(INFO, "The JobManager was successfully serialized ...");

	// restore state to one equivalent to the original
	// creating a new type Job object
	LOG(INFO, "De-serialize the JobManager ...");
	sdpa::daemon::JobManager::ptr_t pJobMan2;
	try {
		// open the archive
		std::ifstream ifs(filename.c_str());
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<sdpa::daemon::JobManager*>(NULL));
		ia.register_type(static_cast<JobFSM*>(NULL));
		// restore the schedule from the archive
		ia>>pJobMan2;
	} catch(std::exception& ) {
		LOG(ERROR, "Exception during de-serialization process occurred!");
		return;
	}

	LOG(DEBUG, pJobMan2->print());
}

BOOST_AUTO_TEST_SUITE_END()
