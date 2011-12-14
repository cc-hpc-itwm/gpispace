#ifndef TESTS_SDPA_PERFORMANCE_TEST_SC_HPP
#define TESTS_SDPA_PERFORMANCE_TEST_SC_HPP 1

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include <tests/sdpa/PerformanceTestBSCEvent.hpp>

namespace sdpa { namespace tests {
  namespace sc = boost::statechart;
  
  struct S0;
  struct S1;

  struct PerformanceTestBSC : sc::state_machine<PerformanceTestBSC, S0> {};

  struct S0 : sc::simple_state<S0, PerformanceTestBSC> {
    typedef sc::custom_reaction< PerformanceTestBSCEvent > reactions;

    sc::result react(const PerformanceTestBSCEvent &) {
      return transit< S1 >();
    }
  };
  struct S1 : sc::simple_state<S1, PerformanceTestBSC> {
    typedef sc::custom_reaction< PerformanceTestBSCEvent > reactions;

    sc::result react(const PerformanceTestBSCEvent &) {
      return transit< S0 >();
    }
  };
}}

#endif
