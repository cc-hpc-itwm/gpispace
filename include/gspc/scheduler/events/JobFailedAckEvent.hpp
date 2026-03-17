// Copyright (C) 2010,2013-2015,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/JobEvent.hpp>


  namespace gspc::scheduler::events
  {
    class JobFailedAckEvent : public JobEvent
    {
    public:
      using Ptr = std::shared_ptr<JobFailedAckEvent>;

      using JobEvent::JobEvent;

      void handleBy
        (gspc::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleJobFailedAckEvent (source, this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (JobFailedAckEvent)
  }
