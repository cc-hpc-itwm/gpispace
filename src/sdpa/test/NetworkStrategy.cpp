// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sdpa/test/NetworkStrategy.hpp>

namespace sdpa
{
  namespace test
  {
    fhg::com::p2p::address_t NetworkStrategy::connect_to_TESTING_ONLY
      (fhg::com::host_t const& host, fhg::com::port_t const& port)
    {
      return _peer.connect_to (host, port);
    }
  }
}
