/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TESTXMLUTILS_H_
#define TESTXMLUTILS_H_
//cppunit
#include <cppunit/extensions/HelperMacros.h>

#include <gwdl/XMLUtils.h>

namespace gwdl {
  namespace tests {
    class TestXMLUtils : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( gwdl::tests::TestXMLUtils);
      CPPUNIT_TEST( testStartsWith );
      CPPUNIT_TEST( testEndsWith );
      CPPUNIT_TEST( testGetText );
      CPPUNIT_TEST_SUITE_END();

    protected:
      void testStartsWith() ; 
      void testEndsWith() ;
      void testGetText() ;
    };
  }
}

#endif /*TESTXMLUTILS_H_*/
