#ifndef SEDA_TESTS_STAGE_TEST_HPP
#define SEDA_TESTS_STAGE_TEST_HPP 1

#include <cppunit/extensions/HelperMacros.h>

namespace seda {
  namespace tests {
    class SedaStageTest : public CppUnit::TestFixture {
      CPPUNIT_TEST_SUITE( seda::tests::SedaStageTest );
      //      CPPUNIT_TEST_EXCEPTION( testStart_illegal_URI_Throws, cms::CMSException );
      CPPUNIT_TEST( testSendFoo );
      //      CPPUNIT_TEST( testSendReply );
      //      CPPUNIT_TEST_EXCEPTION( testStart_Timeout_Throws, cms::CMSException );
      CPPUNIT_TEST_SUITE_END();

    public:
      void setUp();
      void tearDown();

    protected:
      void testSendFoo();
    };
  }
}

#endif // !SEDA_TESTS_STAGE_TEST_HPP
