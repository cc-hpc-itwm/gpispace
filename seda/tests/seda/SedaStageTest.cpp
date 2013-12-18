#include "SedaStageTest.hpp"

#include <string>
#include <iostream>

#include <seda/Stage.hpp>
#include <seda/IEvent.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/EventCountStrategy.hpp>
#include <seda/DiscardStrategy.hpp>
#include <seda/ForwardStrategy.hpp>

using namespace seda::tests;

namespace
{
  struct dummy_event : seda::IEvent
  {
    virtual std::string str() const { return "dummy"; }
  };

  seda::Stage::Ptr createStage
    (const std::string &name, seda::Strategy::Ptr strategy, int pool_size)
  {
    seda::Stage::Ptr stage (new seda::Stage (name, strategy, pool_size));
    seda::StageRegistry::instance().insert (stage);
    return stage;
  }
}

CPPUNIT_TEST_SUITE_REGISTRATION( SedaStageTest );

void
SedaStageTest::setUp() {}

void
SedaStageTest::tearDown() {
    StageRegistry::instance().clear(); // remove all registered stages
}

void
SedaStageTest::testSendFoo() {
    seda::Strategy::Ptr discard(new seda::DiscardStrategy());
    seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
    discard = seda::Strategy::Ptr(ecs);
    seda::Stage::Ptr stage(createStage("discard", discard, 2));

    stage->start();

    const std::size_t numMsgs(1000);
    for (std::size_t i=0; i < numMsgs; ++i) {
        stage->send(seda::IEvent::Ptr(new dummy_event));
    }

    stage->waitUntilEmpty();
    ecs->wait(numMsgs, 1000);

    CPPUNIT_ASSERT_EQUAL(std::size_t(0), stage->size());
    CPPUNIT_ASSERT_EQUAL(numMsgs, ecs->count());

    stage->stop();
}

void
SedaStageTest::testStartStop() {
    seda::Strategy::Ptr discard(new seda::DiscardStrategy());
    seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
    discard = seda::Strategy::Ptr(ecs);
    seda::Stage::Ptr stage(createStage("discard", discard, 2));

    const std::size_t numMsgs(10);

    stage->start();

    for (std::size_t i=0; i < numMsgs; ++i) {
        stage->send(seda::IEvent::Ptr(new dummy_event));
    }
    stage->waitUntilEmpty(100);
    ecs->wait(numMsgs, 1000);

    CPPUNIT_ASSERT_EQUAL(std::size_t(0), stage->size());
    CPPUNIT_ASSERT_EQUAL(numMsgs, ecs->count());

    stage->stop();

    ecs->reset();

    stage->start();
    for (std::size_t i=0; i < numMsgs; ++i) {
        stage->send(seda::IEvent::Ptr(new dummy_event));
    }
    stage->waitUntilEmpty(100);
    ecs->wait(numMsgs, 1000);

    CPPUNIT_ASSERT_EQUAL(std::size_t(0), stage->size());
    CPPUNIT_ASSERT_EQUAL(numMsgs, ecs->count());

    stage->stop();
}

void
SedaStageTest::testForwardEvents() {
    seda::Strategy::Ptr discard(new seda::DiscardStrategy());
    seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
    discard = seda::Strategy::Ptr(ecs);
    seda::Stage::Ptr final(createStage("final", discard, 2));

    seda::Strategy::Ptr fwdStrategy(new seda::ForwardStrategy("final"));
    seda::Stage::Ptr first(createStage("first", fwdStrategy, 1));

    seda::StageRegistry::instance().startAll();

    const std::size_t numMsgs(5);
    for (std::size_t i=0; i < numMsgs; ++i) {
        first->send(seda::IEvent::Ptr(new dummy_event));
    }

    first->waitUntilEmpty(100);
    final->waitUntilEmpty(100);
    ecs->wait(numMsgs, 1000);

    CPPUNIT_ASSERT(first->empty());
    CPPUNIT_ASSERT(final->empty());
    CPPUNIT_ASSERT_EQUAL(numMsgs, ecs->count());

    seda::StageRegistry::instance().stopAll();
}
