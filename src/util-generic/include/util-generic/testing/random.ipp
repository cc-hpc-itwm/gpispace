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

#include <util-generic/testing/random/bool.hpp>
#include <util-generic/testing/random/char.hpp>
#include <util-generic/testing/random/floating_point.hpp>
#include <util-generic/testing/random/hard_integral_typedef.hpp>
#include <util-generic/testing/random/integral.hpp>
#include <util-generic/testing/random/string.hpp>

// \todo Not needed by this header, but a lot of test cases rely on
// this fixing a missing include that was resolved in 1.65. Add the
// missing include to all tests doing that, or bump Boost and remove
// this check.
#include <boost/version.hpp>
#if BOOST_VERSION < 106500
#include <boost/test/tree/test_unit.hpp>
#endif

#include <algorithm>
#include <unordered_set>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      template<typename Container, typename Generator>
        Container randoms (std::size_t n, Generator&& generator)
      {
        Container container (n);
        std::generate (container.begin(), container.end(), generator);
        return container;
      }

      //! \note will busy-stall when no more unique values can be generated
      template<typename T, typename Generator>
        struct unique_random
      {
        T operator()()
        {
          while (true)
          {
            T value (_generator());
            if (_seen.emplace (value).second)
            {
              return value;
            }
          }
        }

      private:
        Generator _generator;
        std::unordered_set<T> _seen;
      };

      template<typename Container>
        Container unique_randoms (std::size_t count)
      {
        return randoms<Container>
          (count, unique_random<typename Container::value_type>{});
      }
    }
  }
}
