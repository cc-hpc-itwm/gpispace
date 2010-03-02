#include <cppunit/extensions/HelperMacros.h>

namespace sdpa {
  namespace tests {
    class ModuleTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE(sdpa::tests::ModuleTest);
      CPPUNIT_TEST( testModuleLoad );
      CPPUNIT_TEST( testModuleUnLoad );
      CPPUNIT_TEST( testModuleFunctionCall );
      CPPUNIT_TEST( testModuleIllegalFunctionCall );
      CPPUNIT_TEST( testModuleLoopingCall );
      CPPUNIT_TEST( testAddFunctionCall );
      CPPUNIT_TEST( testAlloc );
      CPPUNIT_TEST( testUpdate );
      CPPUNIT_TEST_SUITE_END();

    public:
      ModuleTest() {}
      virtual ~ModuleTest() {}

      void setUp();
      void tearDown();

    protected:
      void testModuleLoad();
      void testModuleUnLoad();
      void testModuleFunctionCall();
      void testModuleIllegalFunctionCall();
      void testModuleLoopingCall();
      void testAddFunctionCall();
      void testAlloc();
      void testUpdate();
    };
  }
}
