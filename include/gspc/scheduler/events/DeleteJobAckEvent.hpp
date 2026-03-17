// Copyright (C) 2010,2013-2015,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/JobEvent.hpp>


  namespace gspc::scheduler::events
  {
    class DeleteJobAckEvent : public JobEvent
    {
    public:
      using Ptr = std::shared_ptr<DeleteJobAckEvent>;

      using JobEvent::JobEvent;

      void handleBy
        (gspc::com::p2p::address_t const&, EventHandler*) override
      {
        throw std::runtime_error
          ( "This method should never be called as only the client should handle"
            " this event and it does not use the EventHandler interface."
          );
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (DeleteJobAckEvent)
  }
