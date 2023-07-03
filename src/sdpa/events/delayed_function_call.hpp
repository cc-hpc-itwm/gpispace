// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/events/SDPAEvent.hpp>

#include <functional>

namespace sdpa
{
  namespace events
  {
    class delayed_function_call : public sdpa::events::SDPAEvent
    {
    public:
      delayed_function_call (std::function<void()> function)
        : SDPAEvent()
        , _function (function)
      {}

      void handleBy
        (fhg::com::p2p::address_t const&, EventHandler*) override
      {
        _function();
      }

    private:
      std::function<void()> _function;
    };

    //! \note No serialization: Shall only be used within daemon, not over net
  }
}
