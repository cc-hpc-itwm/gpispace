// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/events/EventHandler.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class SubmitJobAckEvent : public JobEvent
    {
    public:
      using Ptr = ::boost::shared_ptr<SubmitJobAckEvent>;

      using JobEvent::JobEvent;

      void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleSubmitJobAckEvent (source, this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (SubmitJobAckEvent)
  }
}
