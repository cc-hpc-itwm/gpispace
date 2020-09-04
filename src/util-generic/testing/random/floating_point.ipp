// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <util-generic/testing/random/integral.hpp>

#include <climits>
#include <cmath>
#include <cstddef>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        template<std::size_t Bits> struct uint_with_bits;
        template<> struct uint_with_bits<8> { using type = uint8_t; };
        template<> struct uint_with_bits<16> { using type = uint16_t; };
        template<> struct uint_with_bits<32> { using type = uint32_t; };
        template<> struct uint_with_bits<64> { using type = uint64_t; };

        template<typename T>
          inline bool is_desired_random_float (T x)
        {
          auto const c (std::fpclassify (x));
          return c == FP_ZERO || c == FP_NORMAL;
        }

        template<typename T>
          inline T
            random_impl<T, typename std::enable_if<std::is_floating_point<T>{}>::type>
              ::operator()() const
        {
          //! \note while uniform_real_distribution<T> of
          //! [numeric_limits<T>::lowest(), numeric_limits<T>::max()]
          //! would be preferable, §26.5.8.2.2.2 [rand.dist.uni.real]
          //! defines b - a <= numeric_limits<T>::max(). Thus, we roll
          //! an integral and reinterpret it as float, checking if we
          //! hit undesired values and roll until we have a normal or
          //! zero float. Note that uniform_real_distribution also is
          //! assuming linear representation, which floats don't have,
          //! so would roll extremely high numbers most of the time
          //! due to distribution of floats.

          union
          {
            typename uint_with_bits<sizeof (T) * CHAR_BIT>::type as_int;
            T as_float;
          };

          do
          {
            as_int = random<decltype (as_int)>{}();
          }
          while (!detail::is_desired_random_float (as_float));

          return as_float;
        }
      }

      template<typename T>
        T random_floating_point()
      {
        return random<T>{}();
      }
    }
  }
}
