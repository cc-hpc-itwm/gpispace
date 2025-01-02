// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhgcom/header.hpp>

namespace fhg
{
  namespace com
  {
    namespace p2p
    {
      namespace
      {
        static const int HELLO_PACKET = 0x8;
      }

      header_t::header_t (address_t src_, address_t dst_)
        : src (src_)
        , dst (dst_)
      {}
      header_t::header_t (Hello, address_t src_, address_t dst_)
        : type_of_msg {HELLO_PACKET}
        , src (src_)
        , dst (dst_)
      {}
      bool header_t::is_hello() const
      {
        return type_of_msg == HELLO_PACKET;
      }
    }
  }
}
