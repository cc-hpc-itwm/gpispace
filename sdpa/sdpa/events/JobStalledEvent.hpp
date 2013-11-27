#ifndef SDPA_JOB_STALLED_EVENT_HPP
#define SDPA_JOB_STALLED_EVENT_HPP 1

#include <string>

#include <sdpa/types.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class JobStalledEvent : public sdpa::events::JobEvent
    {
    public:
      typedef boost::shared_ptr<JobEvent> Ptr;

      JobStalledEvent ( const address_t& a_from
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
        handler->handleJobStalledEvent (this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (JobStalledEvent)
  }
}

#endif
