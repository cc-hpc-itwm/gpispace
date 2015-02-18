#pragma once

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/Serialization.hpp>

namespace sdpa
{
  namespace events
  {
    class CancelJobAckEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<CancelJobAckEvent> Ptr;

      using JobEvent::JobEvent;

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleCancelJobAckEvent (source, this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (CancelJobAckEvent)
  }
}
