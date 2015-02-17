#pragma once

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa
{
  namespace events
  {
    class QueryJobStatusEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<QueryJobStatusEvent> Ptr;

      using JobEvent::JobEvent;

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleQueryJobStatusEvent (source, this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (QueryJobStatusEvent)
  }
}
