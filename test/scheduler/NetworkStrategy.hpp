// Copyright (C) 2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/com/NetworkStrategy.hpp>


  namespace gspc::scheduler::test
  {
    class NetworkStrategy : public com::NetworkStrategy
    {
    public:
      using com::NetworkStrategy::NetworkStrategy;

      gspc::com::p2p::address_t connect_to_TESTING_ONLY
        (gspc::com::host_t const&, gspc::com::port_t const&);
    };
  }
