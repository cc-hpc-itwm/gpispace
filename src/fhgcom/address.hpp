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

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace fhg
{
  namespace com
  {
    namespace p2p
    {
      struct address_t
      {
        explicit
        address_t ();

        address_t (std::string const& name);

        std::uint8_t data[16];
      } __attribute__((packed));

      // standard operators
      bool operator==(address_t const& lhs, address_t const& rhs);
      bool operator!=(address_t const& lhs, address_t const& rhs);
      bool operator< (address_t const& lhs, address_t const& rhs);

      std::string to_string_TESTING_ONLY (address_t const& a);
      std::size_t hash_value (address_t const& address);
    }
  }
}

namespace std
{
  template<> struct hash<fhg::com::p2p::address_t>
  {
    size_t operator()(fhg::com::p2p::address_t const&) const;
  };
}
