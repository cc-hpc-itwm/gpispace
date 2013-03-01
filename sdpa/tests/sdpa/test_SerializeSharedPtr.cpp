/* =====================================================================================
 *
 *       Filename:  test_SerializeSharedPtr.cpp
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
 * ===================================================================================== */
#define BOOST_TEST_MODULE TestSerializeSharedPtr
#include <sdpa/daemon/JobFSM.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <cstddef>
#include <fstream>
#include <string>
#include <cstdio>
#include <boost/config.hpp>
//#include <boost/archive/tmpdir.hpp>
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

#include <boost/serialization/export.hpp>
#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"

using namespace std;

class Test
{
public:
	 static int count;
	 Test(string arg) { x_ = arg; ++count; }
	 Test(){
		 ++count;
	 }

	 virtual ~Test(){
		 --count;
	 }

	 virtual string str() { return x_ + " (Base)"; ; }

private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int /* file version */)
	{
    	ar & BOOST_SERIALIZATION_NVP(x_);
	}

protected:
    string x_;
};

BOOST_SERIALIZATION_SHARED_PTR(Test)

class Derived : public Test
{
public:
    static int count;
    Derived() : Test() {}
    Derived(std::string arg) : Test(arg) { }
    virtual string str() { return x_ + " (Derived)"; }

    virtual ~Derived() {};

private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /* file_version */){
        ar & boost::serialization::base_object<Test>(*this);
    }
};

BOOST_SERIALIZATION_SHARED_PTR(DERIVED)


int Test::count = 0;

struct MyFixture
{
	MyFixture() : SDPA_INIT_LOGGER("sdpa.tests.TestSerializeSharedPtr") {}
	~MyFixture(){}

	SDPA_DECLARE_LOGGER();
};

BOOST_FIXTURE_TEST_SUITE( test_SerializeSharedPtr, MyFixture )

BOOST_AUTO_TEST_CASE(testSerializeBoostShPtrToTxt)
{
	/*std::string filename(boost::archive::tmpdir());
	filename += "testArchShPtr.txt";*/

	std::cout<<"Testing boost::shared_ptr serialization into txt file ..."<<std::endl;
	std::string filename = "testSerializeBoostShPtr2Txt.txt";

	boost::shared_ptr<Test> shptrS(new Test("a_string"));
	{
		std::ofstream ofs(filename.c_str());
		boost::archive::text_oarchive oa(ofs);
		oa << BOOST_SERIALIZATION_NVP(shptrS);
		ofs.flush();
	}

	shptrS.reset();

	boost::shared_ptr<Test> shptrD;
	{
		std::ifstream ifs(filename.c_str());
		boost::archive::text_iarchive ia(ifs);
		ia >> BOOST_SERIALIZATION_NVP(shptrD);
		cout<<"The deserialized shared pointer contains the value "<<shptrD->str()<<endl;
	}

	shptrD.reset();
	std::cout << std::endl;
}


BOOST_AUTO_TEST_CASE(testSerializeBoostShPtrToXml)
{
	/*std::string filename(boost::archive::tmpdir());
	filename += "testArchShPtr.txt";*/

	std::cout<<"Testing boost::shared_ptr serialization into xml file ..."<<std::endl;
	std::string filename = "testSerializeBoostShPtr2Xml.xml";

	boost::shared_ptr<Test> shptrS(new Test("a_string"));
	{
		std::ofstream ofs(filename.c_str());
		boost::archive::xml_oarchive oa(ofs);
		oa << BOOST_SERIALIZATION_NVP(shptrS);
		ofs.flush();
	}
	shptrS.reset();
	
	boost::shared_ptr<Test> shptrD;
	{
		std::ifstream ifs(filename.c_str());
		boost::archive::xml_iarchive ia(ifs);
		ia >> BOOST_SERIALIZATION_NVP(shptrD);
		cout<<"The deserialized shared pointer contains the value "<<shptrD->str()<<endl;
	}
	shptrD.reset();
	
	std::cout << std::endl;
}


void infoPtrs(boost::shared_ptr<Test> &ptrSh1, boost::shared_ptr<Test> &ptrSh2)
{
    std::cout << "ptrSh1 = 0x" << std::hex << ptrSh1.get() << " ";
    if (ptrSh1.get()) std::cout << "is a " << typeid(*(ptrSh1.get())).name() << "* ";
    std::cout << "use count = " << std::dec << ptrSh1.use_count() << std::endl;
    std::cout << "ptrSh2 = 0x" << std::hex << ptrSh2.get() << " ";
    if (ptrSh2.get()) std::cout << "is a " << typeid(*(ptrSh2.get())).name() << "* ";
    std::cout << "use count = " << std::dec << ptrSh2.use_count() << std::endl;
    std::cout << "unique element count = " << Test::count << std::endl;
}

BOOST_AUTO_TEST_CASE(testSerializeBoostShPtrDerived)
{
	std::cout<<"Testing the serialization of a boost::shared_ptr to a derived class into a file ..."<<std::endl;
    std::string filename = "testSerializeBoostShPtrDerived.txt"; // = boost::archive::tmpdir());filename += "/testfile";

    // create  a new shared pointer to ta new object of type Test
    boost::shared_ptr<Test> ptrSh1(new Test);
    boost::shared_ptr<Test> ptrSh2;
    ptrSh2 = ptrSh1;
    infoPtrs(ptrSh1, ptrSh2);
    // serialize it
    {
        std::ofstream ofs(filename.c_str());
        boost::archive::text_oarchive oa(ofs);
        oa << ptrSh1;
        oa << ptrSh2;
    }
    // reset the shared pointer to NULL
    // thereby destroying the object of type Test
    ptrSh1.reset();
    ptrSh2.reset();
    infoPtrs(ptrSh1, ptrSh2);
    // restore state to one equivalent to the original
    // creating a new type Test object
    {
        // open the archive
        std::ifstream ifs(filename.c_str());
        boost::archive::text_iarchive ia(ifs);

        // restore the schedule from the archive
        ia >> ptrSh1;
        ia >> ptrSh2;
    }
    infoPtrs(ptrSh1, ptrSh2);
    ptrSh1.reset();
    ptrSh2.reset();

    // create  a new shared pointer to ta new object of type Test
    ptrSh1 = boost::shared_ptr<Test>(new Derived);
    ptrSh2 = ptrSh1;
    infoPtrs(ptrSh1, ptrSh2);
    // serialize it
    {
        std::ofstream ofs(filename.c_str());
        boost::archive::text_oarchive oa(ofs);
        oa.register_type(static_cast<Derived *>(NULL));
        oa << ptrSh1;
        oa << ptrSh2;
    }

    // reset the shared pointer to NULL
    // thereby destroying the object of type Derived
    ptrSh1.reset();
    ptrSh2.reset();
    infoPtrs(ptrSh1, ptrSh2);
    // restore state to one equivalent to the original
    // creating a new type Derived object
    {
        // open the archive
        std::ifstream ifs(filename.c_str());
        boost::archive::text_iarchive ia(ifs);

        // restore the schedule from the archive
        ia.register_type(static_cast<Derived *>(NULL));
        ia >> ptrSh1;
        ia >> ptrSh2;
    }
    infoPtrs(ptrSh1, ptrSh2);
}

BOOST_AUTO_TEST_CASE(testSerializeNormalPtr )
{
	std::cout<<"Testing serialization of an object pointed by a normal pointer into a text file ..."<<std::endl;
    std::string filename = "testSerializeNormalPtr.txt"; // = boost::archive::tmpdir());filename += "/testfile";

    // create  a new shared pointer to ta new object of type Test
    Test* p1 = new Test("testSerializeNormalPtr");
    // serialize it
    {
        std::ofstream ofs(filename.c_str());
        boost::archive::text_oarchive oa(ofs);
        oa << p1;
    }

    // restore state to one equivalent to the original
    // creating a new type Test object
    Test* p2 = NULL;
    {
        // open the archive
        std::ifstream ifs(filename.c_str());
        boost::archive::text_iarchive ia(ifs);

        // restore the schedule from the archive
        ia >> p2;
    }

    std::cout<<"The restored value of the pointer is "<<p2->str()<<endl<<std::endl;

	delete p1;
	delete p2;

	std::cout<<"Testing serialization of an object pointed by a normal pointer into an xml file ..."<<std::endl;
    filename = "testSerializeNormalPtr.xml"; // = boost::archive::tmpdir());filename += "/testfile";

	// create  a new shared pointer to ta new object of type Test
	p1 = new Test("testSerializeNormalPtr");
	// serialize it
	{
		std::ofstream ofs(filename.c_str());
		boost::archive::xml_oarchive oa(ofs);
		oa << BOOST_SERIALIZATION_NVP(p1);
	}

	// restore state to one equivalent to the original
	// creating a new type Test object
	p2 = NULL;
	{
		// open the archive
		std::ifstream ifs(filename.c_str());
		boost::archive::xml_iarchive ia(ifs);

		// restore the schedule from the archive
		ia >> BOOST_SERIALIZATION_NVP(p2);
	}

	std::cout<<"The restored value of the pointer is "<<p2->str()<<std::endl;
	delete p1; delete p2;

	std::cout<<"Testing serialization of an object pointed by a normal pointer into a binary file ..."<<std::endl;
	filename = "testNormalPtr.bin"; // = boost::archive::tmpdir());filename += "/testfile";

	// create  a new shared pointer to ta new object of type Test
	p1 = new Test("testSerializeNormalPtr");
	// serialize it
	{
	   std::ofstream ofs(filename.c_str());
	   boost::archive::binary_oarchive oa(ofs);
	   oa << p1;
	}

	// restore state to one equivalent to the original
	// creating a new type Test object
	p2 = NULL;
	{
	   // open the archive
	   std::ifstream ifs(filename.c_str());
	   boost::archive::binary_iarchive ia(ifs);

	   // restore the schedule from the archive
	   ia >> p2;
	}

	std::cout<<"The restored value of the pointer is "<<p2->str()<<std::endl;
	delete p1; delete p2;


	std::cout<<"Testing serialization of an object pointed by a normal pointer into a binary file ..."<<std::endl;
    filename = "testSerializeNormalPtr2Derived.txt"; // = boost::archive::tmpdir());filename += "/testfile";

    p1 = new Derived("testSerializeNormalPtr2Derived");

    // serialize it
	{
	   std::ofstream ofs(filename.c_str());
	   boost::archive::text_oarchive oa(ofs);
	   oa.register_type(static_cast<Derived *>(NULL));
	   oa << p1;
	}

	// restore state to one equivalent to the original
	// creating a new type Test object
	p2 = NULL;
	{
	   // open the archive
	   std::ifstream ifs(filename.c_str());
	   boost::archive::text_iarchive ia(ifs);
	   ia.register_type(static_cast<Derived *>(NULL));
	   // restore the schedule from the archive
	   ia >> p2;
	}

	std::cout<<"The restored value of the pointer is "<<p2->str()<<std::endl;
	delete p1; delete p2;
}

BOOST_AUTO_TEST_CASE(testSerializeMapPtr)
{
	std::cout<<"Test map of pointers serialization ..."<<std::endl;
    std::string filename = "testSerializeMapPtr.txt"; // = boost::archive::tmpdir());filename += "/testfile";

    std::map<std::string, Test*> mapPtrsOut;

    mapPtrsOut["1"] = new Derived("first");
    mapPtrsOut["2"] = new Test("second");
    mapPtrsOut["3"] = new Test("third");
    mapPtrsOut["4"] = new Derived("fourth");
    mapPtrsOut["5"] = new Test("fifth");

    // serialize it
	{
	   std::ofstream ofs(filename.c_str());
	   boost::archive::text_oarchive oa(ofs);
	   oa.register_type(static_cast<Derived *>(NULL));
	   oa << BOOST_SERIALIZATION_NVP(mapPtrsOut);
	}

	for(  std::map<std::string, Test*>::iterator it = mapPtrsOut.begin(); it !=  mapPtrsOut.end(); it++  )
		if(it->second)
			delete it->second;

	// restore state to one equivalent to the original
	// creating a new type Test object
	std::map<std::string, Test*> mapPtrsIn;
	{
	   // open the archive
	   std::ifstream ifs(filename.c_str());
	   boost::archive::text_iarchive ia(ifs);
	   ia.register_type(static_cast<Derived *>(NULL));
	   // restore the schedule from the archive
	   ia >> BOOST_SERIALIZATION_NVP(mapPtrsIn);
	}

	for(  std::map<std::string, Test*>::iterator iter = mapPtrsIn.begin(); iter !=  mapPtrsIn.end(); iter++  )
		std::cout<<"mapPtrIn["<<iter->first<<"] = "<<iter->second->str()<<std::endl;

	for(  std::map<std::string, Test*>::iterator iter = mapPtrsIn.begin(); iter !=  mapPtrsIn.end(); iter++  )
		if(iter->second)
			delete iter->second;
}

BOOST_AUTO_TEST_SUITE_END()
