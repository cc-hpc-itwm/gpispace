// Copyright (C) 2019,2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/hard_integral_typedef.hpp>
#include <gspc/testing/random/impl.hpp>



    namespace gspc::testing
    {
      namespace detail
      {
        template<typename T, typename>
        struct random_impl;
      }

      template<typename T>
      using random = detail::random_impl<T, void>;
    }



GSPC_TESTING_RANDOM_SPECIALIZE_FULL
  (T, gspc::util::hit_detail::enable_if_hit<T>, typename T)
{
  return T {gspc::testing::random<typename T::underlying_type>{}()};
}
