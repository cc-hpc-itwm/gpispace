#include "TimerTest.hpp"

#include <string>
#include <iostream>

#include <seda/Stage.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/Timer.hpp>
#include <seda/TimerEvent.hpp>

#include <seda/DiscardStrategy.hpp>
#include <seda/EventCountStrategy.hpp>
#include <seda/LoggingStrategy.hpp>

using namespace seda::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( TimerTest );


void
TimerTest::setUp() {}

void
TimerTest::tearDown() {
    StageRegistry::instance().clear(); // remove all registered stages
}

void
TimerTest::testTimer() {
  seda::Strategy::Ptr discard(new seda::DiscardStrategy());
  seda::EventCountStrategy::Ptr ecs(new seda::EventCountStrategy(discard));
  discard = seda::Strategy::Ptr(new seda::LoggingStrategy(discard));
  discard = seda::Strategy::Ptr(ecs);
  seda::Stage::Ptr stage(new seda::Stage("discard", discard, 1));

  seda::StageRegistry::instance().insert(stage);

  seda::Timer::Ptr timer(new seda::Timer("discard",
                                         boost::posix_time::millisec(500)));

  stage->start();
  timer->start();

  ecs->wait(2, 5000); // wait for two events, maximum of 2 seconds
  timer->stop();

  stage->waitUntilEmpty();

  CPPUNIT_ASSERT_EQUAL(std::size_t(2), ecs->count());

  stage->stop();
}

void
TimerTest::testStartStop() {
}
