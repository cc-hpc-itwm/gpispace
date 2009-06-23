#ifndef TESTS_SDPA_PERFORMANCE_TEST_SC_HPP
#define TESTS_SDPA_PERFORMANCE_TEST_SC_HPP 1

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>

namespace tests { namespace sdpa {
  namespace sc = boost::statechart;
  
  struct S0;
  struct S1;

  struct PerformanceTestBSC : sc::state_machine<PerformanceTestBSC, S0> {};

  struct S0 : sc::simple_state<S0, PerformanceTestBSC> {};
  struct S1 : sc::simple_state<S1, PerformanceTestBSC> {};
}}

#endif
