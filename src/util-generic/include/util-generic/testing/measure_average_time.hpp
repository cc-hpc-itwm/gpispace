// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
