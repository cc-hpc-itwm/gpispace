#pragma once

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa
{
  namespace events
  {
    class SubmitJobAckEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<SubmitJobAckEvent> Ptr;

      using JobEvent::JobEvent;

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleSubmitJobAckEvent (source, this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (SubmitJobAckEvent)
  }
}
