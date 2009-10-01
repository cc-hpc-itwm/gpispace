/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TESTPROPERTIES_H_
#define TESTPROPERTIES_H_
//cppunit
#include <cppunit/extensions/HelperMacros.h>
//gwdl
#include <gwdl/Properties.h>

namespace gwdl {
  namespace tests {
    class PropertiesTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( gwdl::tests::PropertiesTest );
      CPPUNIT_TEST( testProperties );
      CPPUNIT_TEST_SUITE_END();

    protected:
      void testProperties();
    };
  }
}

#endif /*TESTPROPERTIES_H_*/
