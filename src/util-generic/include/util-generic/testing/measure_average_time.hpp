// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      template <typename Duration, typename Func>
        Duration measure_average_time (Func fun, std::size_t const count)
      {
        if (0 == count)
        {
          throw std::invalid_argument ("count has to be positive");
        }

        const std::chrono::high_resolution_clock::time_point start
          (std::chrono::high_resolution_clock::now());

        for (std::size_t i (0); i < count; ++i)
        {
          fun();
        }

        return std::chrono::duration_cast<Duration>
          (std::chrono::high_resolution_clock::now() - start) / count;
      }
    }
  }
}
