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

#include <fhgcom/address.hpp>

#include <cstdint>

namespace fhg
{
  namespace com
  {
    namespace p2p
    {
      struct header_t
      {
        header_t (address_t src, address_t dst);

        struct Hello{};

        header_t (Hello, address_t src, address_t dst);

        bool is_hello() const;

        std::uint32_t type_of_msg {0};
        std::uint32_t length {0};  // size of payload in bytes
        address_t src;             // unique source address
        address_t dst;             // unique destination address
      };

      static_assert (40ul == sizeof (header_t), "size of fhg::com::p2p::header_t");
    }
  }
}
