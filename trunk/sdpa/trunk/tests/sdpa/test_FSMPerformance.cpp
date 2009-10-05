#include <iostream>
#include <stdexcept>

#include "test_FSMPerformance.hpp"
#include "PerformanceTestFSMEvent.hpp"
#include "PerformanceTestBSCEvent.hpp"

#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wunused"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "PerformanceTest_sm.h"
#include "PerformanceTest_sc.h"
#include <sdpa/util.hpp>

using namespace sdpa;
using namespace sdpa::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( sdpa::tests::FSMPerformanceTest );

void FSMPerformanceTest::setUp() {
}

void FSMPerformanceTest::tearDown() {
}

void FSMPerformanceTest::testSMCPerformance() {
  FSMPerformanceTestContext fsm(*this);

  const std::size_t numTransitions(1000000);

  sdpa::util::time_type start(sdpa::util::now());
  for (std::size_t i = 0; i < numTransitions; ++i) {
    fsm.T( PerformanceTestFSMEvent("0") );
  }
  sdpa::util::time_type delta(sdpa::util::time_diff(start, sdpa::util::now()));

  std::cout << "fsm: " << delta << "us" << std::endl;
}

void FSMPerformanceTest::testBoostStatechartPerformance() {
  PerformanceTestBSC fsm;
  fsm.initiate();

  const std::size_t numTransitions(1000000);

  sdpa::util::time_type start(sdpa::util::now());
  for (std::size_t i = 0; i < numTransitions; ++i) {
    fsm.process_event( PerformanceTestBSCEvent("0") );
  }
  sdpa::util::time_type delta(sdpa::util::time_diff(start, sdpa::util::now()));

  std::cout << "bsc: " << delta << "us" << std::endl;
}

void FSMPerformanceTest::testSMCException() {
  FSMPerformanceTestContext fsm(*this);

  fsm.T( PerformanceTestFSMEvent("0") ); // ok
  try {
    fsm.F( PerformanceTestFSMEvent("error") ); // should throw
    CPPUNIT_ASSERT_MESSAGE("expected std::runtime_error", false);
  } catch (const std::runtime_error &ex) {
    // ok
  }
  fsm.T( PerformanceTestFSMEvent("2") ); // should be ok again
  CPPUNIT_ASSERT_EQUAL(0, fsm.getState().getId());
}

void FSMPerformanceTest::do_s0_s1(const PerformanceTestFSMEvent &e) {
//  std::cout << "FSM: " << "S0" << " --" << e.tag() << "--> " << "S1" << std::endl;
}

void FSMPerformanceTest::do_s1_s0(const PerformanceTestFSMEvent &e) {
//  std::cout << "FSM: "<< "S1" << "--" << e.tag() << "-->" << "S0" << std::endl;
}

void FSMPerformanceTest::do_throw_exception() {
  throw std::runtime_error("exception from within transition test");
}

