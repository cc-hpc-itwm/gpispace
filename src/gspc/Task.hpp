#pragma once

#include <util-generic/hard_integral_typedef.hpp>

namespace gspc
{
  namespace task
  {
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (ID, std::uint64_t);

    struct State; // success, failure (reason), cancelled (reason), ...
  }

  struct Task
  {
  };
}

FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH (gspc::task::ID);
