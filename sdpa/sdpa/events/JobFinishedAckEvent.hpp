#ifndef SDPA_JOB_FINISHED_ACK_EVENT_HPP
#define SDPA_JOB_FINISHED_ACK_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class JobFinishedAckEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<JobFinishedAckEvent> Ptr;

      JobFinishedAckEvent()
        : JobEvent ("", "", "")
      {}

      JobFinishedAckEvent ( const address_t& a_from
                          , const address_t& a_to
                          , const sdpa::job_id_t& a_job_id
                          )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
      {}

      std::string str() const
      {
        return "JobFinishedAckEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleJobFinishedAckEvent (this);
      }
    };
  }
}

#endif
