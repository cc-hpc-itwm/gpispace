// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhgcom/message.hpp>

#include <fhgcom/connection.hpp>

namespace fhg
{
  namespace com
  {
    message_t::message_t (p2p::address_t src, p2p::address_t dst)
      : header (src, dst)
    {}
    message_t::message_t (Hello, p2p::address_t src, p2p::address_t dst)
      : header (p2p::header_t::Hello{}, src, dst)
    {}

    bool message_t::is_hello() const
    {
      return header.is_hello();
    }

    void message_t::resize()
    {
      data.resize (header.length);
    }

    message_t::message_t (std::string const& x, p2p::address_t src, p2p::address_t dst)
      : header (src, dst)
      , data (x.begin(), x.end())
    {
      header.length = data.size();
    }
  }
}
