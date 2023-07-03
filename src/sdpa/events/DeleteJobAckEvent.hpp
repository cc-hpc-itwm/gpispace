// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class DeleteJobAckEvent : public JobEvent
    {
    public:
      using Ptr = ::boost::shared_ptr<DeleteJobAckEvent>;

      using JobEvent::JobEvent;

      void handleBy
        (fhg::com::p2p::address_t const&, EventHandler*) override
      {
        throw std::runtime_error
          ( "This method should never be called as only the client should handle"
            " this event and it does not use the EventHandler interface."
          );
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (DeleteJobAckEvent)
  }
}
