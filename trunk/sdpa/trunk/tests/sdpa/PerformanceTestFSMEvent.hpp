#ifndef TESTS_SDPA_PERFORMANCETESTFSMEVENT_HPP
#define TESTS_SDPA_PERFORMANCETESTFSMEVENT_HPP 1

#include <string>

namespace sdpa { namespace tests {
  class PerformanceTestFSMEvent {
  public:
    explicit
    PerformanceTestFSMEvent(const std::string &tag)
      : tag_(tag) {}

    const std::string &tag() const { return tag_; }
  private:
    std::string tag_;
  };
}}

#endif
