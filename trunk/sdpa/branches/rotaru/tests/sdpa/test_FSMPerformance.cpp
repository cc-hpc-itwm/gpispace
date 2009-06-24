#include <iostream>

#include "test_FSMPerformance.hpp"
#include "PerformanceTestFSMEvent.hpp"
#include "PerformanceTestBSCEvent.hpp"
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

void FSMPerformanceTest::do_s0_s1(const PerformanceTestFSMEvent &e) {
//  std::cout << "FSM: " << "S0" << " --" << e.tag() << "--> " << "S1" << std::endl;
}

void FSMPerformanceTest::do_s1_s0(const PerformanceTestFSMEvent &e) {
//  std::cout << "FSM: "<< "S1" << "--" << e.tag() << "-->" << "S0" << std::endl;
}

