#include "SedaStageTest.hpp"

#include <string>
#include <iostream>

#include <seda/Stage.hpp>
#include <seda/IEvent.hpp>
#include <seda/EventCountStrategy.hpp>

using namespace seda::tests;

namespace
{
  struct dummy_event : seda::IEvent
  {
    virtual std::string str() const { return "dummy"; }
  };

  struct discard_strategy : public Strategy
  {
    discard_strategy() : Strategy ("discard") {}
    void perform (const IEvent::Ptr&) {}
  };
}

CPPUNIT_TEST_SUITE_REGISTRATION( SedaStageTest );

void
SedaStageTest::setUp() {}

void
SedaStageTest::tearDown() {}

void
SedaStageTest::testSendFoo() {
    seda::Strategy::Ptr discard(new discard_strategy);
    seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
    discard = seda::Strategy::Ptr(ecs);
    seda::Stage::Ptr stage(seda::Stage::Ptr (new seda::Stage ("discard", discard)));

    stage->start();

    const std::size_t numMsgs(1000);
    for (std::size_t i=0; i < numMsgs; ++i) {
        stage->send(seda::IEvent::Ptr(new dummy_event));
    }

    ecs->wait(numMsgs);

    CPPUNIT_ASSERT_EQUAL(numMsgs, ecs->count());

    stage->stop();
}

void
SedaStageTest::testStartStop() {
    seda::Strategy::Ptr discard(new discard_strategy);
    seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
    discard = seda::Strategy::Ptr(ecs);
    seda::Stage::Ptr stage(seda::Stage::Ptr (new seda::Stage("discard", discard)));

    const std::size_t numMsgs(10);

    stage->start();

    for (std::size_t i=0; i < numMsgs; ++i) {
        stage->send(seda::IEvent::Ptr(new dummy_event));
    }
    ecs->wait(numMsgs);

    CPPUNIT_ASSERT_EQUAL(numMsgs, ecs->count());

    stage->stop();

    ecs->reset();

    stage->start();
    for (std::size_t i=0; i < numMsgs; ++i) {
        stage->send(seda::IEvent::Ptr(new dummy_event));
    }
    ecs->wait(numMsgs);

    CPPUNIT_ASSERT_EQUAL(numMsgs, ecs->count());

    stage->stop();
}
