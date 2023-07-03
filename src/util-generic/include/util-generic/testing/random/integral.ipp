// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
