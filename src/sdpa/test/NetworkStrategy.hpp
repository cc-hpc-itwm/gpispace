// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/com/NetworkStrategy.hpp>

namespace sdpa
{
  namespace test
  {
    class NetworkStrategy : public com::NetworkStrategy
    {
    public:
      using com::NetworkStrategy::NetworkStrategy;

      fhg::com::p2p::address_t connect_to_TESTING_ONLY
        (fhg::com::host_t const&, fhg::com::port_t const&);
    };
  }
}
