#pragma once

#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class BacklogNoLongerFullEvent : public MgmtEvent
    {
    public:
      typedef boost::shared_ptr<BacklogNoLongerFullEvent> Ptr;

      BacklogNoLongerFullEvent() : MgmtEvent(){}

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleBacklogNoLongerFullEvent (source, this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_MGMTEVENT_OVERLOAD (BacklogNoLongerFullEvent)
  }
}
