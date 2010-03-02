/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TESTOPERATION_H_
#define TESTOPERATION_H_
#include <cppunit/extensions/HelperMacros.h>

#include <gwdl/Operation.h>

namespace gwdl {
  namespace tests {
    class OperationTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( gwdl::tests::OperationTest );
      CPPUNIT_TEST( testOperation );
      CPPUNIT_TEST_SUITE_END();

      protected:
      void testOperation() ;
    };
  }
}

#endif /*TESTOPERATION_H_*/
