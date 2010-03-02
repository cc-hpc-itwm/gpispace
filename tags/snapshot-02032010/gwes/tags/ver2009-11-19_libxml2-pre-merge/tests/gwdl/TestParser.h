/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef PARSERDATA_H_
#define PARSERDATA_H_
#include <cppunit/extensions/HelperMacros.h>

namespace gwdl {
  namespace tests {
    class ParserTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( gwdl::tests::ParserTest );
      CPPUNIT_TEST( testParser );
      CPPUNIT_TEST_SUITE_END();

    protected:
      void testParser() ;
    };
  }
}

#endif /*PARSERDATA_H_*/
