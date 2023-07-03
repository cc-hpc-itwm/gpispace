// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
