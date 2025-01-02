// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/random/char.hpp>

#include <util-generic/testing/random.hpp>

#include <cstddef>
#include <numeric>
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
        std::string const& random_impl<char>::any()
        {
          static auto const any_chars
            = []
              {
                std::string chars (256, '\0');
                std::iota (chars.begin(), chars.end(), 0);
                return chars;
              }();

          return any_chars;
        }

        std::string const& random_impl<char>::any_without_zero()
        {
          static auto const any_without_zero_chars
            = []
              {
                std::string chars (255, '\0');
                std::iota (chars.begin(), chars.end(), 1);
                return chars;
              }();

          return any_without_zero_chars;
        }

#define PRECONDITION(cond_)                                             \
        if (!(cond_))                                                   \
        {                                                               \
          throw std::logic_error                                        \
            ("random<char>: precondition failed: " #cond_);             \
        }

        char random_impl<char>::operator() (std::string const& chars) const
        {
          PRECONDITION (!chars.empty())

          return chars.at (random<std::size_t>{} (chars.size() - 1, 0));
        }

#undef PRECONDITION
      }

      char random_char_of (std::string const& chars)
      {
        return random<char>{} (chars);
      }
    }
  }
}
