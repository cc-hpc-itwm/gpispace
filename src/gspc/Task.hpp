#pragma once

#include <util-generic/hard_integral_typedef.hpp>

namespace gspc
{
  namespace task
  {
    // more complex:
    // - hierarchy with user_context
    // - information for scheduler
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (ID, std::uint64_t);

    struct State; // success (result), failure (reason), cancelled (reason), ...
  }

  struct Task
  {
  };
}

FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH (gspc::task::ID);
