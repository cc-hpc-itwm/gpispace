#include "EventQueueTest.hpp"

#include <string>

#include <seda/EventQueue.hpp>
#include <seda/UserEvent.hpp>

using namespace seda::tests;

namespace
{
  struct dummy_event : seda::IEvent
  {
    virtual std::string str() const { return "dummy"; }
  };
}

CPPUNIT_TEST_SUITE_REGISTRATION( EventQueueTest );

EventQueueTest::EventQueueTest()
  : SEDA_INIT_LOGGER("tests.seda.EventQueueTest")
{}

void
EventQueueTest::setUp() {
  _queue = seda::EventQueue::Ptr(new seda::EventQueue("tests.seda.testqueue", 5));
}

void
EventQueueTest::tearDown() {
  // remove all remaining events
  _queue->clear();
}

void
EventQueueTest::testPushPop() {
  CPPUNIT_ASSERT(_queue->empty());

  seda::IEvent::Ptr in(new dummy_event);
  _queue->push(in);
  CPPUNIT_ASSERT(!_queue->empty());
  CPPUNIT_ASSERT_EQUAL((std::size_t)1, _queue->size());

  seda::IEvent::Ptr out(_queue->pop());
  CPPUNIT_ASSERT( in == out );
  CPPUNIT_ASSERT_EQUAL((std::size_t)0, _queue->size());
  CPPUNIT_ASSERT(_queue->empty());
}
