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

#include <cstdint>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      //! On-the-wire prefix for messages. Where used, this header is
      //! followed by a byte blob of size `buffer_size`.
      struct packet_header
      {
        std::uint64_t message_id;
        std::uint64_t buffer_size;

        packet_header() = default;
        packet_header (std::uint64_t id, std::uint64_t size)
          : message_id (id)
          , buffer_size (size)
          {}
      };
    }
  }
}
