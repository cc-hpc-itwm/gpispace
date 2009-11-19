/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TESTPLACE_H_
#define TESTPLACE_H_
//cppunit
#include <cppunit/extensions/HelperMacros.h>
#include <gwdl/Place.h>
//fhglog
#include <fhglog/fhglog.hpp>

namespace gwdl {
  namespace tests {
    class PlaceTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( gwdl::tests::PlaceTest );
      CPPUNIT_TEST( testPlace );
      CPPUNIT_TEST_SUITE_END();

    protected:
      void testPlace() ;
      void printTokens(fhg::log::logger_t logger, gwdl::Place &place); 
    };
  }
}


#endif /*TESTPLACE_H_*/
