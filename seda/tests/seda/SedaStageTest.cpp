#include "SedaStageTest.hpp"

#include <string>
#include <iostream>

#include <seda/Stage.hpp>
#include <seda/EventQueue.hpp>
#include <seda/IEvent.hpp>
#include <seda/StageFactory.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/EventCountStrategy.hpp>
#include <seda/DiscardStrategy.hpp>
#include <seda/ForwardStrategy.hpp>
#include <seda/StringEvent.hpp>

using namespace seda::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( SedaStageTest );

void
SedaStageTest::setUp() {}

void
SedaStageTest::tearDown() {
    StageRegistry::instance().clear(); // remove all registered stages
}

void
SedaStageTest::testSendFoo() {
    seda::StageFactory::Ptr factory(new seda::StageFactory());

    seda::Strategy::Ptr discard(new seda::DiscardStrategy());
    seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
    discard = seda::Strategy::Ptr(ecs);
    seda::Stage::Ptr stage(factory->createStage("discard", discard, 2));

    stage->start();

    const std::size_t numMsgs(1000);
    for (std::size_t i=0; i < numMsgs; ++i) {
        stage->send(seda::IEvent::Ptr(new seda::StringEvent("foo")));
    }

    stage->waitUntilEmpty();
    ecs->wait(numMsgs, 1000);

    CPPUNIT_ASSERT_EQUAL(std::size_t(0), stage->size());
    CPPUNIT_ASSERT_EQUAL(numMsgs, ecs->count());

    stage->stop();
}

void
SedaStageTest::testStartStop() {
    seda::StageFactory::Ptr factory(new seda::StageFactory());

    seda::Strategy::Ptr discard(new seda::DiscardStrategy());
    seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
    discard = seda::Strategy::Ptr(ecs);
    seda::Stage::Ptr stage(factory->createStage("discard", discard, 2));

    const std::size_t numMsgs(10);

    stage->start();

    for (std::size_t i=0; i < numMsgs; ++i) {
        stage->send(seda::IEvent::Ptr(new seda::StringEvent("foo")));
    }
    stage->waitUntilEmpty(100);
    ecs->wait(numMsgs, 1000);

    CPPUNIT_ASSERT_EQUAL(std::size_t(0), stage->size());
    CPPUNIT_ASSERT_EQUAL(numMsgs, ecs->count());

    stage->stop();

    ecs->reset();

    stage->start();
    for (std::size_t i=0; i < numMsgs; ++i) {
        stage->send(seda::IEvent::Ptr(new seda::StringEvent("bar")));
    }
    stage->waitUntilEmpty(100);
    ecs->wait(numMsgs, 1000);

    CPPUNIT_ASSERT_EQUAL(std::size_t(0), stage->size());
    CPPUNIT_ASSERT_EQUAL(numMsgs, ecs->count());

    stage->stop();
}

void
SedaStageTest::testForwardEvents() {
    seda::StageFactory::Ptr factory(new seda::StageFactory());

    seda::Strategy::Ptr discard(new seda::DiscardStrategy());
    seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
    discard = seda::Strategy::Ptr(ecs);
    seda::Stage::Ptr final(factory->createStage("final", discard, 2));

    seda::Strategy::Ptr fwdStrategy(new seda::ForwardStrategy("final"));
    seda::Stage::Ptr first(factory->createStage("first", fwdStrategy));

    seda::StageRegistry::instance().startAll();

    const std::size_t numMsgs(5);
    for (std::size_t i=0; i < numMsgs; ++i) {
        first->send(seda::IEvent::Ptr(new seda::StringEvent("foo")));
    }

    first->waitUntilEmpty(100);
    final->waitUntilEmpty(100);
    ecs->wait(numMsgs, 1000);

    CPPUNIT_ASSERT(first->empty());
    CPPUNIT_ASSERT(final->empty());
    CPPUNIT_ASSERT_EQUAL(numMsgs, ecs->count());

    seda::StageRegistry::instance().stopAll();
}
