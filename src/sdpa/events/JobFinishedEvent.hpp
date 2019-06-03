#pragma once

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

#include <we/type/activity.hpp>
#include <we/type/net.hpp>

#include <fhg/util/boost/optional.hpp>

namespace sdpa
{
  namespace events
  {
    class JobFinishedEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<JobFinishedEvent> Ptr;

      JobFinishedEvent ( const sdpa::job_id_t& a_job_id
                       , const sdpa::finished_reason_t& reason
                       )
        : sdpa::events::JobEvent (a_job_id)
        , _reason (std::move(reason))
      {}

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleJobFinishedEvent (source, this);
      }

      const sdpa::finished_reason_t& reason() const
      {
        return _reason;
      }

    private:
      sdpa::finished_reason_t _reason;

    };

    SAVE_CONSTRUCT_DATA_DEF (JobFinishedEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->reason());
    }

    LOAD_CONSTRUCT_DATA_DEF (JobFinishedEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (job_id);
      LOAD_FROM_ARCHIVE (sdpa::finished_reason_t, reason);
      ::new (e) JobFinishedEvent(job_id, reason);
    }


  }
}
