#pragma once

#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class WorkerRegistrationAckEvent : public MgmtEvent
    {
    public:
      typedef boost::shared_ptr<WorkerRegistrationAckEvent> Ptr;

      using MgmtEvent::MgmtEvent;

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleWorkerRegistrationAckEvent (source, this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_MGMTEVENT_OVERLOAD (WorkerRegistrationAckEvent)
  }
}
