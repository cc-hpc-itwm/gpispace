// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
