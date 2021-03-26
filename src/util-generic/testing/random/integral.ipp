// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <util-generic/testing/random.hpp>

#include <random>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
#define PRECONDITION(cond_)                                             \
        if (!(cond_))                                                   \
        {                                                               \
          throw std::logic_error                                        \
            ("random<integral>: precondition failed: " #cond_);         \
        }

        template<typename T>
          T random_impl<T, typename std::enable_if<non_char_integral<T>{}>::type>
            ::operator() (T max, T min) const
        {
          PRECONDITION (max >= min)

          std::uniform_int_distribution<T> distribution {min, max};
          return distribution (GLOBAL_random_engine());
        }

        template<typename T>
          T random_impl<T, typename std::enable_if<non_char_integral<T>{}>::type>
            ::operator() (non_zero, T max, T min) const
        {
          T x (0);
          while (x == 0)
          {
            x = random<T>{} (max, min);
          }
          return x;
        }

#undef PRECONDITION
      }

      template<typename T>
        T random_integral()
      {
        return random<T>{}();
      }

      template<typename T>
        T random_integral_without_zero()
      {
        return random<T>{} (typename random<T>::non_zero{});
      }
    }
  }
}
