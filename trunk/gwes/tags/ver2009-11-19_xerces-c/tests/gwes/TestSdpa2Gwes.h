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

class TestSdpa2Gwes : public CPPUNIT_NS::TestFixture {
	CPPUNIT_TEST_SUITE( gwes::tests::TestSdpa2Gwes );
	CPPUNIT_TEST( testWorkflowWithSdpaActivity );
	CPPUNIT_TEST( testWorkflowWithSdpaSubWorkflow );
	CPPUNIT_TEST_SUITE_END();

protected:
	void testWorkflowWithSdpaActivity();
	void testWorkflowWithSdpaSubWorkflow();
	
}; // end class TestSdpa2Gwes  

} // end namespace tests
} // end namespace gwes

#endif /*TESTSDPA2GWES_H_*/
