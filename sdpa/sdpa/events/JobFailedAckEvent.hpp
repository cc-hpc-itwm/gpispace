#ifndef SDPA_JOB_FAILED_ACK_EVENT_HPP
#define SDPA_JOB_FAILED_ACK_EVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class JobFailedAckEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<JobFailedAckEvent> Ptr;

      using JobEvent::JobEvent;

      virtual void handleBy
        (std::string const& source, EventHandler* handler) override
      {
        handler->handleJobFailedAckEvent (source, this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (JobFailedAckEvent)
  }
}

#endif
