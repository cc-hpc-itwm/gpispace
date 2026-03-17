// Copyright (C) 2010,2014-2016,2018,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/com/address.hpp>

#include <cstdint>



    namespace gspc::com::p2p
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

      static_assert (40ul == sizeof (header_t), "size of gspc::com::p2p::header_t");
    }
