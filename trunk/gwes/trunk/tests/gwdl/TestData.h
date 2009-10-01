/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TESTDATA_H_
#define TESTDATA_H_
#include <cppunit/extensions/HelperMacros.h>

#include <gwdl/Data.h>

namespace gwdl {
  namespace tests {
    class DataTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( gwdl::tests::DataTest );
      CPPUNIT_TEST( testData );
      CPPUNIT_TEST_SUITE_END();

    protected:
      void testData() ;
    };
  }
}

#endif /*TESTDATA_H_*/
