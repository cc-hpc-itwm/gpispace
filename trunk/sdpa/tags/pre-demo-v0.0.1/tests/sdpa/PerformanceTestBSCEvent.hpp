#ifndef TESTS_SDPA_PERFORMANCETESTBSCEVENT_HPP
#define TESTS_SDPA_PERFORMANCETESTBSCEVENT_HPP 1

#include <string>
#include <boost/statechart/event.hpp>

namespace sdpa { namespace tests {
  namespace sc = boost::statechart;

  struct PerformanceTestBSCEvent : sc::event<PerformanceTestBSCEvent> {
    PerformanceTestBSCEvent(const std::string &tag) : tag_(tag) {}
    std::string tag_;
  };
}}

#endif
