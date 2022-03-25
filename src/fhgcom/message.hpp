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
#include <fhgcom/header.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace fhg
{
  namespace com
  {
    struct message_t
    {
      message_t (p2p::address_t src, p2p::address_t dst);

      struct Hello{};

      message_t (Hello, p2p::address_t src, p2p::address_t dst);

      bool is_hello() const;

      void resize();

      message_t (std::string const&, p2p::address_t src, p2p::address_t dst);

      p2p::header_t header;
      std::vector<char> data;
    };
  }
}
