// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhgcom/peer_info.hpp>

namespace fhg
{
  namespace com
  {
    host_t::host_t (std::string const& s)
      : _hostname (s)
    {}
    host_t::operator std::string() const
    {
      return _hostname;
    }
  }
}
