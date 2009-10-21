#include <cppunit/extensions/HelperMacros.h>

namespace sdpa {
  namespace tests {
    class ActivityExecutorTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE(sdpa::tests::ActivityExecutorTest);
      CPPUNIT_TEST_SUITE_END();

    public:
      ActivityExecutorTest() {}
      virtual ~ActivityExecutorTest() {}

      void setUp();
      void tearDown();

    protected:
    };
  }
}
