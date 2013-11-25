#ifndef SDPA_JOB_RUNNING_EVENT_HPP
#define SDPA_JOB_RUNNING_EVENT_HPP 1

#include <string>

#include <sdpa/memory.hpp>
#include <sdpa/types.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class JobRunningEvent : public sdpa::events::JobEvent
    {
    public:
      typedef sdpa::shared_ptr<JobEvent> Ptr;

      JobRunningEvent ( const address_t& a_from
               , const address_t& a_to
               , const sdpa::job_id_t& a_job_id
               )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
      {}

      std::string str() const
      {
        return "JobStalledEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleJobRunningEvent (this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (JobRunningEvent)
  }
}

#endif
