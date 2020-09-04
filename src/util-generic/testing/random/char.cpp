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
