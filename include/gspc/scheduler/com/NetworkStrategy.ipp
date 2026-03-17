// Copyright (C) 2019,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <utility>


  namespace gspc::scheduler::com
  {
    template<typename Event, typename... Args>
      void NetworkStrategy::perform
        (gspc::com::p2p::address_t const& address, Args&&... args)
    {
      return perform
        (address, _codec.encode<Event> (std::forward<Args> (args)...));
    }
  }
