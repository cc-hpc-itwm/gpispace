#pragma once

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

#include <we/type/activity.hpp>
#include <we/type/net.hpp>

namespace sdpa
{
  namespace events
  {
    class JobFinishedEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<JobFinishedEvent> Ptr;

      JobFinishedEvent ( const sdpa::job_id_t& a_job_id
                       , we::type::activity_t job_result
                       )
        : sdpa::events::JobEvent (a_job_id)
        , result_ (std::move (job_result))
      {}

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleJobFinishedEvent (source, this);
      }

      const we::type::activity_t& result() const
      {
        return result_;
      }

    private:
      we::type::activity_t result_;
    };

    SAVE_CONSTRUCT_DATA_DEF (JobFinishedEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->result());
    }

    LOAD_CONSTRUCT_DATA_DEF (JobFinishedEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (job_id);
      LOAD_FROM_ARCHIVE (we::type::activity_t, result);

      ::new (e) JobFinishedEvent (job_id, std::move (result));
    }
  }
}
