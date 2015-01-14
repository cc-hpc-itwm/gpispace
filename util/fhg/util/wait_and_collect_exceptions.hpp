#ifndef FHG_UTIL_WAIT_AND_COLLECT_EXCEPTIONS_HPP
#define FHG_UTIL_WAIT_AND_COLLECT_EXCEPTIONS_HPP

#include <future>
#include <vector>

namespace fhg
{
  namespace util
  {
    void wait_and_collect_exceptions (std::vector<std::future<void>>&);
  }
}

#endif
