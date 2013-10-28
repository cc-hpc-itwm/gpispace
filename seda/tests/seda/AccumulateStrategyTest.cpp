#include "AccumulateStrategyTest.hpp"
#include <seda/DiscardStrategy.hpp>
#include <seda/StringEvent.hpp>
#include <typeinfo>

using namespace seda::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( AccumulateStrategyTest );

AccumulateStrategyTest::AccumulateStrategyTest() 
  : SEDA_INIT_LOGGER("tests.seda.AccumulateStrategyTest") {
    seda::DiscardStrategy::Ptr discard(new seda::DiscardStrategy());
  _accumulate = seda::AccumulateStrategy::Ptr(new seda::AccumulateStrategy(discard));
  }

void AccumulateStrategyTest::setUp() {
}

void AccumulateStrategyTest::tearDown() {
  // remove all remaining events
  _accumulate->clear();
}

void AccumulateStrategyTest::testAddRemoveEvents() {
  SEDA_LOG_DEBUG("Testing add events");
  CPPUNIT_ASSERT(_accumulate->empty());
  IEvent::Ptr event(new StringEvent("foobar"));
  _accumulate->perform(event);
  SEDA_LOG_DEBUG("Testing accumulator size (size = "<<_accumulate->size() <<")");
  CPPUNIT_ASSERT_EQUAL((std::size_t) 1, _accumulate->size());
  SEDA_LOG_DEBUG("Testing clear of accumulator (size = "<<_accumulate->size() <<")");
  _accumulate->clear();
  CPPUNIT_ASSERT_EQUAL((std::size_t) 0, _accumulate->size());
  SEDA_LOG_DEBUG("Accumulator size = "<<_accumulate->size() <<" after calling clear");
}

void AccumulateStrategyTest::testCheckSequence() {
  SEDA_LOG_DEBUG("Testing check of sequence.");
  CPPUNIT_ASSERT(_accumulate->empty());
  IEvent::Ptr event(new StringEvent("foobar"));
  _accumulate->perform(event);
  std::list<std::string> expectedSequence;
  expectedSequence.push_back(typeid(StringEvent).name());
  CPPUNIT_ASSERT_EQUAL(true, _accumulate->checkSequence(expectedSequence));
  _accumulate->clear();
  CPPUNIT_ASSERT_EQUAL(false, _accumulate->checkSequence(expectedSequence));
}

void AccumulateStrategyTest::testIterator() {
  const std::size_t acc_size=100;
  CPPUNIT_ASSERT(_accumulate->empty());
  SEDA_LOG_DEBUG("Filling Accumulator with " << acc_size << " IEvents.");
  for (std::size_t i=0; i < acc_size; i++) {
    IEvent::Ptr event(new StringEvent("foobar" ));
    _accumulate->perform(event);
  }
  SEDA_LOG_DEBUG("Accumulator size is " << _accumulate->size() << " IEvents.");
  CPPUNIT_ASSERT_EQUAL(acc_size, _accumulate->size());
  std::size_t counter=0;
  seda::AccumulateStrategy::iterator it;
  for (it=_accumulate->begin();
       it != _accumulate->end();
       it++) {
    // We iterate over the type 
    //  typedef std::list<IEvent::Ptr>::iterator iterator_type;
    // -> dereference the iterator in order to get the IEvent::Ptr...
    SEDA_LOG_DEBUG("Found Event: " << (*it)->str());
    counter++;
  }
  SEDA_LOG_DEBUG("Iterated over " << counter << " IEvents.");
  CPPUNIT_ASSERT_EQUAL(acc_size, counter);
}
