/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TESTSDPA2GWES_H_
#define TESTSDPA2GWES_H_
//cppunit
#include <cppunit/extensions/HelperMacros.h>

namespace gwes {
namespace tests {

class Sdpa2GwesAPITest : public CPPUNIT_NS::TestFixture {
	CPPUNIT_TEST_SUITE( gwes::tests::Sdpa2GwesAPITest );
	CPPUNIT_TEST( testWorkflowWithSdpaActivity );
	//CPPUNIT_TEST( testSdpa2Gwes );
	//CPPUNIT_TEST( testGwes2Sdpa );
	CPPUNIT_TEST_SUITE_END();

protected:
	void testWorkflowWithSdpaActivity();
	void testSdpa2Gwes();
	void testGwes2Sdpa();
	
}; // end class Sdpa2GwesAPITest  

} // end namespace tests
} // end namespace gwes

#endif /*TESTSDPA2GWES_H_*/
