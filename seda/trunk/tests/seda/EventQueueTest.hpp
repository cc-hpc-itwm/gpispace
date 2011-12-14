#ifndef SEDA_TESTS_EVENTQUEUE_HPP
#define SEDA_TESTS_EVENTQUEUE_HPP 1

#include <cppunit/extensions/HelperMacros.h>
#include <seda/common.hpp>
#include <seda/IEventQueue.hpp>

namespace seda {
  namespace tests {
    class EventQueueTest : public CppUnit::TestFixture {
      CPPUNIT_TEST_SUITE( seda::tests::EventQueueTest );
      CPPUNIT_TEST( testPushPop );
      CPPUNIT_TEST( testQueueEmpty );
      CPPUNIT_TEST_SUITE_END();

    private:

    public:
      EventQueueTest();
      void setUp();
      void tearDown();

    protected:
      SEDA_DECLARE_LOGGER();
      void testPushPop();
      void testQueueEmpty();

      seda::IEventQueue::Ptr _queue;
    };
  }
}

#endif // !SEDA_TESTS_EVENTQUEUE_HPP
