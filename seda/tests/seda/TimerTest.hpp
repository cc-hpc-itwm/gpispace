#ifndef SEDA_TESTS_TIMER_TEST_HPP
#define SEDA_TESTS_TIMER_TEST_HPP 1

#include <seda/common.hpp>
#include <cppunit/extensions/HelperMacros.h>

namespace seda {
  namespace tests {
    class TimerTest : public CppUnit::TestFixture {
      CPPUNIT_TEST_SUITE( seda::tests::TimerTest );
      CPPUNIT_TEST( testTimer );
      CPPUNIT_TEST( testStartStop );
      CPPUNIT_TEST_SUITE_END();

    private:
      SEDA_DECLARE_LOGGER();

    public:
      TimerTest() : SEDA_INIT_LOGGER("TimerTest") {};
      void setUp();
      void tearDown();

    protected:
      void testTimer();
      void testStartStop();
    };
  }
}

#endif // !SEDA_TESTS_TIMER_TEST_HPP
