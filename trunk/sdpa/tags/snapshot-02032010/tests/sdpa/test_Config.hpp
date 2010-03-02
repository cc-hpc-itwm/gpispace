#ifndef TESTS_SDPA_TEST_CONFIG_HPP
#define TESTS_SDPA_TEST_CONFIG_HPP

#include <cppunit/extensions/HelperMacros.h>

namespace sdpa {
  namespace tests {
    class ConfigTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE(sdpa::tests::ConfigTest);
      CPPUNIT_TEST( testPopulate );
      CPPUNIT_TEST_SUITE_END();

    public:
      ConfigTest() {}
      virtual ~ConfigTest() {}

      void setUp();
      void tearDown();

    protected:
      void testPopulate();
    };
  }
}

#endif
