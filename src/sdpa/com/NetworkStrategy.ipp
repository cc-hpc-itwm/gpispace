// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <utility>

namespace sdpa
{
  namespace com
  {
    template<typename Event, typename... Args>
      void NetworkStrategy::perform
        (fhg::com::p2p::address_t const& address, Args&&... args)
    {
      return perform
        (address, _codec.encode<Event> (std::forward<Args> (args)...));
    }
  }
}
