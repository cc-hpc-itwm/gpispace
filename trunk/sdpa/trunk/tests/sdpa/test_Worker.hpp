#include <cppunit/extensions/HelperMacros.h>

namespace sdpa {
  namespace tests {
    class WorkerTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE( sdpa::tests::WorkerTest);
      CPPUNIT_TEST( testDispatch );
      CPPUNIT_TEST( testGetNextJob );
      CPPUNIT_TEST( testAcknowledge );
      CPPUNIT_TEST_SUITE_END();

    private:
    public:
      WorkerTest();
      ~WorkerTest();
      void setUp();
      void tearDown();

    protected:
      void testDispatch();
      void testGetNextJob();
      void testAcknowledge();
    };
  }
}
