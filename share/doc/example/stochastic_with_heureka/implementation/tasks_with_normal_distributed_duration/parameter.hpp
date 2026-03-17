#pragma once

#include <type_traits>

namespace gspc::share::example::stochastic_with_heurake::tasks
{
  struct Parameter
  {
    unsigned int mean;
    unsigned int deviation;
  };
  static_assert (std::is_trivially_copyable_v<Parameter>);
}
