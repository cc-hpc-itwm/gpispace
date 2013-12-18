#include "SedaStageTest.hpp"

#include <string>
#include <iostream>

#include <seda/Stage.hpp>
#include <seda/IEvent.hpp>

using namespace seda::tests;

namespace
{
  struct dummy_event : seda::IEvent
  {
    virtual std::string str() const { return "dummy"; }
  };

  struct wait_for_n_events_strategy : public seda::Strategy
  {
    wait_for_n_events_strategy (unsigned int expected)
      : _counter (0)
      , _expected (expected)
    {}

    void perform (const seda::IEvent::Ptr&)
    {
      boost::mutex::scoped_lock _ (_counter_mutex);
      ++_counter;

      CPPUNIT_ASSERT (_counter <= _expected);
      if (_counter == _expected)
      {
        _expected_count_reached.notify_all();
      }
    }
    void wait() const
    {
      boost::mutex::scoped_lock _ (_counter_mutex);

      while (_counter < _expected)
      {
        _expected_count_reached.wait (_);
      }

      CPPUNIT_ASSERT_EQUAL (_counter, _expected);
    }

    mutable boost::mutex _counter_mutex;
    mutable boost::condition_variable _expected_count_reached;
    unsigned int _counter;
    unsigned int _expected;
  };
}

CPPUNIT_TEST_SUITE_REGISTRATION( SedaStageTest );

void
SedaStageTest::setUp() {}

void
SedaStageTest::tearDown() {}

void
SedaStageTest::testSendFoo() {
    const std::size_t numMsgs(1000);

    wait_for_n_events_strategy* counter (new wait_for_n_events_strategy (numMsgs));
    seda::Strategy::Ptr counter_shared (counter);

    seda::Stage::Ptr stage (seda::Stage::Ptr (new seda::Stage (counter_shared)));
    stage->start();

    for (std::size_t i=0; i < numMsgs; ++i) {
        stage->send(seda::IEvent::Ptr(new dummy_event));
    }

    counter->wait();
}
