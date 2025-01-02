// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/connectable_to_address_string.hpp>

#include <util-generic/hostname.hpp>

namespace fhg
{
  namespace util
  {
    std::string connectable_to_address_string
      (::boost::asio::ip::address const& address)
    {
      if (address.is_unspecified())
      {
        return hostname();
      }
      return address.to_string();
    }
  }
}
