// Copyright (C) 2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/com/peer_info.hpp>


  namespace gspc::com
  {
    host_t::host_t (std::string const& s)
      : _hostname (s)
    {}
    host_t::operator std::string() const
    {
      return _hostname;
    }
  }
