#include "EventQueueTest.hpp"

#include <string>

#include <seda/EventQueue.hpp>
#include <seda/StringEvent.hpp>

using namespace seda::tests;

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

  seda::IEvent::Ptr in(new seda::StringEvent("test"));
  _queue->push(in);
  CPPUNIT_ASSERT(!_queue->empty());
  CPPUNIT_ASSERT_EQUAL((std::size_t)1, _queue->size());

  seda::IEvent::Ptr out(_queue->pop(10));
  CPPUNIT_ASSERT( in == out );
  CPPUNIT_ASSERT_EQUAL((std::size_t)0, _queue->size());
  CPPUNIT_ASSERT(_queue->empty());
}

void
EventQueueTest::testQueueEmpty_Throws() {
  CPPUNIT_ASSERT(_queue->empty());
  _queue->pop(10);
}

void
EventQueueTest::testQueueFull_Throws() {
  // fill the queue
  while (_queue->size() < _queue->maxQueueSize()) {
    _queue->push(seda::IEvent::Ptr(new StringEvent("test")));
  }
  // push one more element to trigger the overflow
  _queue->push(seda::IEvent::Ptr(new StringEvent("test")));
}
