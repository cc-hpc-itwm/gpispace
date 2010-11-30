#ifndef TESTS_SDPA_SUITE_HPP
#define TESTS_SDPA_SUITE_HPP 1

#include <cppunit/TestFixture.h>

#include <tests/sdpa/test_Worker.hpp>
#include <tests/sdpa/test_UUID.hpp>
#include <tests/sdpa/test_JobId.hpp>
#include <tests/sdpa/test_Config.hpp>
//#include <tests/sdpa/test_FSMPerformance.hpp>
#include <tests/sdpa/test_JobFSM_SMC.hpp>

//#include <tests/sdpa/test_Scheduler.hpp>

namespace sdpa { namespace tests {
  class Suite : public CPPUNIT_NS::TestFixture {
  public:
    static CPPUNIT_NS::Test *suite() {
      CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite( "SdpaTestSuite" );

      // disabled for now: suiteOfTests->addTest( ModuleTest::suite() );

      suiteOfTests->addTest( WorkerTest::suite() );
      suiteOfTests->addTest( UUIDTest::suite() );
      suiteOfTests->addTest( JobIdTest::suite() );
      suiteOfTests->addTest( ConfigTest::suite() );

      //suiteOfTests->addTest( SchedulerTest::suite() );
      suiteOfTests->addTest( JobFSMTest_SMC::suite() );
      //suiteOfTests->addTest( LoadBalancerTest::suite() );

      return suiteOfTests;
    }
  };
}}

#endif
