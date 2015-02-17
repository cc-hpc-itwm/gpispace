#pragma once

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class JobFinishedAckEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<JobFinishedAckEvent> Ptr;

      using JobEvent::JobEvent;

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleJobFinishedAckEvent (source, this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (JobFinishedAckEvent)
  }
}
