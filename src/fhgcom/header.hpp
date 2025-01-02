// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
