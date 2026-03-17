#pragma once

#include <type_traits>

namespace gspc::share::example::stochastic_with_heurake::pi
{
  struct PartialResult
  {
    unsigned long in;
    unsigned long n;
  };
  static_assert (std::is_trivially_copyable_v<PartialResult>);

  struct Result
  {
    PartialResult result;
    PartialResult reduced;
    double pi;
  };
  static_assert (std::is_trivially_copyable_v<Result>);
}
