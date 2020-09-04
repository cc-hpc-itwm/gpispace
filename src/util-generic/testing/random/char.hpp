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

#pragma once

#include <util-generic/testing/random/impl.hpp>

#include <string>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        template<>
          struct random_impl<char>
        {
          //! 0x00..0xFF
          static std::string const& any();
          //! 0x01..0xFF
          static std::string const& any_without_zero();

          //! Generate a random character choosing from only the
          //! characters given in \a chars.
          char operator() (std::string const& chars = any()) const;
        };
      }

      //! uniformly select a char from \a chars
      //! \note: deprecated Prefer random<char>.
      char random_char_of (std::string const& chars);
    }
  }
}
