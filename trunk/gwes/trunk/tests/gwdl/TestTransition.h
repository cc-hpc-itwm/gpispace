/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TESTTRANSITION_H_
#define TESTTRANSITION_H_
#include <cppunit/extensions/HelperMacros.h>

#include <gwdl/Transition.h>

namespace gwdl {
  namespace tests {
    class TransitionTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( gwdl::tests::TransitionTest );
      CPPUNIT_TEST( testTransition );
      CPPUNIT_TEST_SUITE_END();

    protected:
      void testTransition();
    };
  }
}

#endif /*TESTTRANSITION_H_*/
