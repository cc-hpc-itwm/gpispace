#ifndef SEDA_TESTS_STAGE_TEST_HPP
#define SEDA_TESTS_STAGE_TEST_HPP 1

#include <seda/common.hpp>
#include <cppunit/extensions/HelperMacros.h>

namespace seda {
  namespace tests {
    class SedaStageTest : public CppUnit::TestFixture {
      CPPUNIT_TEST_SUITE( seda::tests::SedaStageTest );
      //      CPPUNIT_TEST_EXCEPTION( testStart_illegal_URI_Throws, cms::CMSException );
      CPPUNIT_TEST( testSendFoo );
      CPPUNIT_TEST( testStartStop );
      CPPUNIT_TEST( testForwardEvents );
      CPPUNIT_TEST( testCompositeStrategy );
      CPPUNIT_TEST( testAccumulateStrategy );
      CPPUNIT_TEST( testLossyDaemonStrategy );
      CPPUNIT_TEST( testFilterStrategy );
      //      CPPUNIT_TEST( testSendReply );
      //      CPPUNIT_TEST_EXCEPTION( testStart_Timeout_Throws, cms::CMSException );
      CPPUNIT_TEST_SUITE_END();

    private:
      SEDA_DECLARE_LOGGER();

    public:
      SedaStageTest() : SEDA_INIT_LOGGER("SedaStageTest") {};
      void setUp();
      void tearDown();

    protected:
      void testSendFoo();
      void testStartStop();
      void testForwardEvents();
      void testCompositeStrategy();
      void testAccumulateStrategy();
      void testLossyDaemonStrategy();
      void testFilterStrategy();
    };
  }
}

#endif // !SEDA_TESTS_STAGE_TEST_HPP
