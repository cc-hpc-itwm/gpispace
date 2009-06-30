#include <cppunit/extensions/HelperMacros.h>
#include <tests/sdpa/PerformanceTestFSMEvent.hpp>
#include <tests/sdpa/PerformanceTestBSCEvent.hpp>

namespace sdpa {
  namespace tests {
    class FSMPerformanceTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE(sdpa::tests::FSMPerformanceTest);
      CPPUNIT_TEST( testSMCPerformance );
      CPPUNIT_TEST( testBoostStatechartPerformance );
      CPPUNIT_TEST( testSMCException );
      CPPUNIT_TEST_SUITE_END();

    public:
      FSMPerformanceTest() {}
      virtual ~FSMPerformanceTest() {}

      void setUp();
      void tearDown();

      // FSM callbacks
      void do_s0_s1(const PerformanceTestFSMEvent &e);
      void do_s1_s0(const PerformanceTestFSMEvent &e);
      void do_throw_exception();

    protected:
      void testSMCPerformance();
      void testBoostStatechartPerformance();
      void testSMCException();
    };
  }
}
