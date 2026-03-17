// Copyright (C) 2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <random>
#include <gspc/testing/random/impl.hpp>

namespace gspc::testing::detail
{
  template<>
    struct random_impl<boost::multiprecision::cpp_int, void>
  {
    boost::multiprecision::cpp_int operator()() const
    {
      // Generate a random cpp_int in the range of long
      std::uniform_int_distribution<long> dist
        ( std::numeric_limits<long>::lowest()
        , std::numeric_limits<long>::max()
        );

      return boost::multiprecision::cpp_int (dist (GLOBAL_random_engine()));
    }
  };
}
