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

      JobFailedAckEvent ( const address_t& a_from
                        , const address_t& a_to
                        , const sdpa::job_id_t& a_job_id
                        )
        :  sdpa::events::JobEvent (a_from, a_to, a_job_id)
      {}

      std::string str() const
      {
        return "JobFailedAckEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleJobFailedAckEvent (this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (JobFailedAckEvent)
  }
}

#endif
