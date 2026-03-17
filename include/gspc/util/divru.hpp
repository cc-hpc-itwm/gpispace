#pragma once

#include <gspc/util/hard_integral_typedef.hpp>

#include <type_traits>


  namespace gspc::util
  {
    template < typename Integral
             , typename = typename std::enable_if
                 < std::is_integral<Integral>::value
                 || hit_detail::is_hit<Integral>::value
                 >::type
             >
      constexpr Integral divru (Integral lhs, Integral rhs)
    {
      return (lhs + rhs - Integral (1)) / rhs;
    }
  }
