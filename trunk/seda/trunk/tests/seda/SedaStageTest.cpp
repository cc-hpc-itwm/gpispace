#include "SedaStageTest.hpp"

#include <string>
#include <iostream>

#include <seda/Stage.hpp>
#include <seda/EventQueue.hpp>
#include <seda/IEvent.hpp>
#include <seda/StageFactory.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/EventCountStrategy.hpp>
#include <seda/CompositeStrategy.hpp>
#include <seda/AccumulateStrategy.hpp>
#include <seda/DiscardStrategy.hpp>
#include <seda/CoutStrategy.hpp>
#include <seda/ForwardStrategy.hpp>
#include <seda/LoggingStrategy.hpp>
#include <seda/LossyDaemonStrategy.hpp>
#include <seda/FilterStrategy.hpp>
#include <seda/TimerEvent.hpp>
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

void
SedaStageTest::testCompositeStrategy() {
    seda::StageFactory::Ptr factory(new seda::StageFactory());

    seda::Strategy::Ptr discard(new seda::DiscardStrategy());
    seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
    discard = seda::Strategy::Ptr(ecs);
    seda::Stage::Ptr final(factory->createStage("discard", discard, 2));

    seda::CompositeStrategy::Ptr composite(new seda::CompositeStrategy("composite"));
    composite->add(seda::Strategy::Ptr(new seda::ForwardStrategy("discard")));
    composite->add(seda::Strategy::Ptr(new seda::ForwardStrategy("discard")));
    seda::Stage::Ptr first(factory->createStage("fwd", composite));

    seda::StageRegistry::instance().startAll();

    first->send(seda::IEvent::Ptr(new seda::StringEvent("foo")));

    first->waitUntilEmpty(100);
    final->waitUntilEmpty(100);
    ecs->wait(2, 1000); // event should have been duplicated

    CPPUNIT_ASSERT(first->empty());
    CPPUNIT_ASSERT(final->empty());
    CPPUNIT_ASSERT_EQUAL((std::size_t)2, ecs->count());

    seda::StageRegistry::instance().stopAll();
}

void SedaStageTest::testAccumulateStrategy() {
    SEDA_LOG_DEBUG("Testing AccumulateStrategy");
    seda::StageFactory::Ptr factory(new seda::StageFactory());

    seda::Strategy::Ptr discard(new seda::DiscardStrategy());
    seda::AccumulateStrategy::Ptr accumulate(new seda::AccumulateStrategy(discard));
    seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(accumulate));
    seda::Stage::Ptr first(factory->createStage("accumulate", ecs));

    first->start();

    seda::IEvent::Ptr sendEvent(new seda::StringEvent("foo"));
    first->send(sendEvent);

    first->waitUntilEmpty(100);
    ecs->wait(1, 1000); 

    CPPUNIT_ASSERT(first->empty());
    CPPUNIT_ASSERT_EQUAL((std::size_t)1, ecs->count());

    // Now, check what we have accumulated in our strategy.
    SEDA_LOG_DEBUG("Expected event: " << sendEvent->str());
    seda::AccumulateStrategy::iterator it;
    for (it = accumulate->begin(); it != accumulate->end(); *it++) {
        SEDA_LOG_DEBUG("Found event: " << (*it)->str());
        if ((*it)->str().compare(sendEvent->str()) != 0) 
            CPPUNIT_FAIL("Expected and actual event differ!");
    }

    first->stop();
}

void SedaStageTest::testLossyDaemonStrategy() {
    SEDA_LOG_DEBUG("Testing LossyDaemonStrategy");
    seda::StageFactory::Ptr factory(new seda::StageFactory());

    seda::Strategy::Ptr discard(new seda::DiscardStrategy());
    seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
    discard = seda::Strategy::Ptr(new seda::LossyDaemonStrategy(ecs, 0.5));
    seda::Stage::Ptr first(factory->createStage("lossy", discard));

    first->start();

    for (size_t i = 0; i < 100; i++) {
        seda::IEvent::Ptr sendEvent(new seda::StringEvent("foo"));
        first->send(sendEvent);
    }
    first->waitUntilEmpty(4000);

    CPPUNIT_ASSERT(first->empty());
    CPPUNIT_ASSERT(std::size_t(0)   < ecs->count());
    CPPUNIT_ASSERT(std::size_t(100) > ecs->count());

    first->stop();
}

void SedaStageTest::testFilterStrategy() {
    SEDA_LOG_DEBUG("Testing FilterStrategy");
    seda::StageFactory::Ptr factory(new seda::StageFactory());
    seda::Strategy::Ptr discard(new seda::DiscardStrategy());
    seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
    discard = seda::Strategy::Ptr(new seda::FilterStrategy<seda::StringEvent>(ecs));
    seda::Stage::Ptr first(factory->createStage("filter", discard));

    first->start();

    // string events are being filtered
    first->send(seda::IEvent::Ptr(new seda::StringEvent("foo")));
    first->waitUntilEmpty(500);

    CPPUNIT_ASSERT(first->empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), ecs->count());

    // timer events are not filtered
    first->send(seda::IEvent::Ptr(new seda::TimerEvent("test-timer")));
    first->waitUntilEmpty(500);

    ecs->wait(1, 1000);

    CPPUNIT_ASSERT(first->empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), ecs->count());

    first->stop();
}

