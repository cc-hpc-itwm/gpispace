#ifndef TESTS_SDPA_TEST_JOB_ID_HPP
#define TESTS_SDPA_TEST_JOB_ID_HPP

#include <cppunit/extensions/HelperMacros.h>

namespace sdpa {
  namespace tests {
    class JobIdTest : public CPPUNIT_NS::TestFixture {
      CPPUNIT_TEST_SUITE(sdpa::tests::JobIdTest);
      CPPUNIT_TEST( testDefaultConstructor );
      CPPUNIT_TEST( testAutoConversionFromString );
      CPPUNIT_TEST( testAutoConversionToString );
      CPPUNIT_TEST_SUITE_END();

    public:
      JobIdTest() {}
      virtual ~JobIdTest() {}

      void setUp();
      void tearDown();

    protected:
      void testDefaultConstructor();
      void testAutoConversionFromString();
      void testAutoConversionToString();
    };
  }
}

#endif
