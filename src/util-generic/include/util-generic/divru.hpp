// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/hard_integral_typedef.hpp>

#include <type_traits>

namespace fhg
{
  namespace util
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
}
