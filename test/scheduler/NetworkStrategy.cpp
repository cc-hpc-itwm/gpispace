// Copyright (C) 2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/scheduler/NetworkStrategy.hpp>


  namespace gspc::scheduler::test
  {
    gspc::com::p2p::address_t NetworkStrategy::connect_to_TESTING_ONLY
      (gspc::com::host_t const& host, gspc::com::port_t const& port)
    {
      return _peer.connect_to (host, port);
    }
  }
