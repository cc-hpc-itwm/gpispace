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

#include <random>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      template<> constexpr std::size_t state_bits<std::mt19937>()
      {
        return std::mt19937::state_size * std::mt19937::word_size;
      }
      template<> constexpr std::size_t state_bits<std::mt19937_64>()
      {
        return std::mt19937_64::state_size * std::mt19937_64::word_size;
      }
      template<> constexpr std::size_t state_bits<std::minstd_rand0>()
      {
        return sizeof (std::minstd_rand0::result_type) * CHAR_BIT;
      }
      template<> constexpr std::size_t state_bits<std::minstd_rand>()
      {
        return sizeof (std::minstd_rand::result_type) * CHAR_BIT;
      }
    }
  }
}
