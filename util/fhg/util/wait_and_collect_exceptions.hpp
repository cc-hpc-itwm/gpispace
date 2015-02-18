#pragma once

#include <future>
#include <vector>

namespace fhg
{
  namespace util
  {
    void wait_and_collect_exceptions (std::vector<std::future<void>>&);
  }
}
