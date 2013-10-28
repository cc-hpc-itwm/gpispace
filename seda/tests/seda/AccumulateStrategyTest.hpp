#ifndef ___TESTS_SEDA_ACCUMULATESTRATEGYTEST_HPP
#define ___TESTS_SEDA_ACCUMULATESTRATEGYTEST_HPP 1

#include <cppunit/extensions/HelperMacros.h>
#include <seda/common.hpp>
#include <seda/AccumulateStrategy.hpp>

#include <boost/utility.hpp>

namespace seda {
  namespace tests {
    class AccumulateStrategyTest : public CppUnit::TestFixture
                                 , boost::noncopyable {
        typedef seda::shared_ptr<AccumulateStrategyTest> Ptr;
        CPPUNIT_TEST_SUITE( seda::tests::AccumulateStrategyTest );
        CPPUNIT_TEST( testAddRemoveEvents );
        CPPUNIT_TEST( testIterator );
        CPPUNIT_TEST( testCheckSequence );
        CPPUNIT_TEST_SUITE_END();

      public:
        AccumulateStrategyTest ();
        void setUp();
        void tearDown();

      protected:
        SEDA_DECLARE_LOGGER();
        void testAddRemoveEvents();
        void testIterator();
        void testCheckSequence();
      private:
        seda::AccumulateStrategy::Ptr _accumulate;

    };
  }
}

#endif /* ___TESTS_SEDA_ACCUMULATESTRATEGYTEST_HPP */
