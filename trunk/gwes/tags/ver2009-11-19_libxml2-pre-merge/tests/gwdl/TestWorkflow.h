/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TESTWORKFLOW_H_
#define TESTWORKFLOW_H_
//cppunit
#include <cppunit/extensions/HelperMacros.h>
//gwdl
#include <gwdl/Workflow.h>

namespace gwdl {
  namespace tests {
    class WorkflowTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( gwdl::tests::WorkflowTest );
      CPPUNIT_TEST( testWorkflow );
      CPPUNIT_TEST_SUITE_END();

    protected:
      void testWorkflow() ;
    };
  }
}

#endif /*TESTWORKFLOW_H_*/
