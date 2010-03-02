#ifndef SEDA_TESTS_COMM_ENCODER_HPP
#define SEDA_TESTS_COMM_ENCODER_HPP 1

#include <cppunit/extensions/HelperMacros.h>
#include <seda/common.hpp>

namespace seda {
  namespace comm {
    namespace tests {
      class EncoderTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE( seda::comm::tests::EncoderTest );
        CPPUNIT_TEST( testEncode );
        CPPUNIT_TEST_SUITE_END();

        private:

        public:
        EncoderTest();
        void setUp();
        void tearDown();

        protected:
        SEDA_DECLARE_LOGGER();
        void testEncode();
      };
    }
  }
}

#endif
