// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/hard_integral_typedef.hpp>
#include <util-generic/testing/random/impl.hpp>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        template<typename T, typename>
        struct random_impl;
      }

      template<typename T>
      using random = detail::random_impl<T, void>;
    }
  }
}

FHG_UTIL_TESTING_RANDOM_SPECIALIZE_FULL
  (T, fhg::util::hit_detail::enable_if_hit<T>, typename T)
{
  return T {fhg::util::testing::random<typename T::underlying_type>{}()};
}
