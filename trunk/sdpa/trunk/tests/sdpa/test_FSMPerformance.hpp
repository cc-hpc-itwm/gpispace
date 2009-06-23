#include <cppunit/extensions/HelperMacros.h>

namespace sdpa {
  namespace tests {
    class FSMPerformanceTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE(sdpa::tests::FSMPerformanceTest);
      CPPUNIT_TEST( testSMCPerformance );
      CPPUNIT_TEST( testBoostStatechartPerformance );
      CPPUNIT_TEST_SUITE_END();

    public:
      FSMPerformanceTest() {}
      virtual ~FSMPerformanceTest() {}

      void setUp();
      void tearDown();

    protected:
      void testSMCPerformance();
      void testBoostStatechartPerformance();
    };
  }
}
