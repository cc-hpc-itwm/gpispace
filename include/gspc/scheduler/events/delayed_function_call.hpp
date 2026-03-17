// Copyright (C) 2014-2016,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/SchedulerEvent.hpp>

#include <functional>


  namespace gspc::scheduler::events
  {
    class delayed_function_call : public gspc::scheduler::events::SchedulerEvent
    {
    public:
      delayed_function_call (std::function<void()> function)
        : SchedulerEvent()
        , _function (function)
      {}

      void handleBy
        (gspc::com::p2p::address_t const&, EventHandler*) override
      {
        _function();
      }

    private:
      std::function<void()> _function;
    };

    //! \note No serialization: Shall only be used within daemon, not over net
  }
