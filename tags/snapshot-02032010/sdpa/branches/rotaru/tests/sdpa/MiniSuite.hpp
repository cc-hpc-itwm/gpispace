#ifndef TESTS_SDPA_SUITE_HPP
#define TESTS_SDPA_SUITE_HPP 1

#include <cppunit/TestFixture.h>

#include <tests/sdpa/test_Module.hpp>
#include <tests/sdpa/test_Token.hpp>
#include <tests/sdpa/test_Worker.hpp>
#include <tests/sdpa/test_UUID.hpp>
#include <tests/sdpa/test_JobId.hpp>
#include <tests/sdpa/test_Config.hpp>
#include <tests/sdpa/test_FSMPerformance.hpp>
#include <tests/sdpa/test_JobFSM_SMC.hpp>

#include <tests/sdpa/test_D2D2DDummyGwes.hpp>
#include <tests/sdpa/test_D2D2DRealGwes.hpp>

#include <tests/sdpa/test_C2D2D2DDummyGwes.hpp>
#include <tests/sdpa/test_C2D2D2DRealGwes.hpp>

#include <tests/sdpa/test_DaemonsWithComm.hpp>
#include <tests/sdpa/test_Components.hpp>

#include <tests/sdpa/test_Scheduler.hpp>

namespace sdpa { namespace tests {
  class MiniSuite : public CPPUNIT_NS::TestFixture {
  public:
    static CPPUNIT_NS::Test *suite() {
      CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite( "MiniTestSuite" );

      suiteOfTests->addTest( D2D2DDummyGwesTest::suite() );
      suiteOfTests->addTest( D2D2DRealGwesTest::suite() );
      suiteOfTests->addTest( C2D2D2DDummyGwesTest::suite() );
	  suiteOfTests->addTest( C2D2D2DRealGwesTest::suite() );
	  suiteOfTests->addTest( DaemonsWithCommTest::suite() );
	  suiteOfTests->addTest( TestComponents::suite() );

      return suiteOfTests;
    }
  };
}}

#endif
