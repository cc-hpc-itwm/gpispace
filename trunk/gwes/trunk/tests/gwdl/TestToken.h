/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TESTTOKEN_H_
#define TESTTOKEN_H_
//cppunit
#include <cppunit/extensions/HelperMacros.h>
//gwdl
#include <gwdl/Token.h>

namespace gwdl {
  namespace tests {
    class TokenTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( gwdl::tests::TokenTest );
      CPPUNIT_TEST( testToken );
      CPPUNIT_TEST_SUITE_END();

    protected:
      void testToken() ;
    };
  }
}

#endif /*TESTTOKEN_H_*/
