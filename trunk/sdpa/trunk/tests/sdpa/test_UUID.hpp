#include <cppunit/extensions/HelperMacros.h>

namespace sdpa {
  namespace tests {
    class UUIDTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE(sdpa::tests::UUIDTest);
      CPPUNIT_TEST( testGenerate );
      CPPUNIT_TEST_SUITE_END();

    public:
      UUIDTest() {}
      virtual ~UUIDTest() {}

      void setUp();
      void tearDown();

    protected:
      void testGenerate();
    };
  }
}
