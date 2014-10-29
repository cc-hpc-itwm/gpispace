#ifndef SDPA_JOB_FINISHED_ACK_EVENT_HPP
#define SDPA_JOB_FINISHED_ACK_EVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class JobFinishedAckEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<JobFinishedAckEvent> Ptr;

      JobFinishedAckEvent ( const address_t& a_from
                          , const sdpa::job_id_t& a_job_id
                          )
        : sdpa::events::JobEvent (a_from, a_job_id)
      {}

      virtual void handleBy (EventHandler* handler) override
      {
        handler->handleJobFinishedAckEvent (this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (JobFinishedAckEvent)
  }
}

#endif
