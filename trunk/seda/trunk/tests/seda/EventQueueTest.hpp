#ifndef SEDA_TESTS_EVENTQUEUE_HPP
#define SEDA_TESTS_EVENTQUEUE_HPP 1

#include <cppunit/extensions/HelperMacros.h>
#include <seda/common.hpp>
#include <seda/IEventQueue.hpp>

namespace seda {
  namespace tests {
    class EventQueueTest : public CppUnit::TestFixture {
      CPPUNIT_TEST_SUITE( seda::tests::EventQueueTest );
      //      CPPUNIT_TEST_EXCEPTION( testStart_illegal_URI_Throws, cms::CMSException );
      CPPUNIT_TEST( testPushPop );
      CPPUNIT_TEST_EXCEPTION( testQueueEmpty_Throws, seda::QueueEmpty );
      CPPUNIT_TEST_EXCEPTION( testQueueFull_Throws, seda::QueueFull );
      CPPUNIT_TEST_SUITE_END();

    private:

    public:
      EventQueueTest();
      void setUp();
      void tearDown();

    protected:
      SEDA_DECLARE_LOGGER();
      void testPushPop();
      void testQueueEmpty_Throws();
      void testQueueFull_Throws();

      seda::IEventQueue::Ptr _queue;
    };
  }
}

#endif // !SEDA_TESTS_EVENTQUEUE_HPP
