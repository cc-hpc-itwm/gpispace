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
                       , const sdpa::task_completed_reason_t& job_result
                       )
        : sdpa::events::JobEvent (a_job_id)
        , _reason (std::move(job_result))
      {}

      JobFinishedEvent ( const sdpa::job_id_t& a_job_id
                       , const sdpa::task_failed_reason_t& error_info
                       )
        : sdpa::events::JobEvent (a_job_id)
        , _reason (std::move(error_info))
      {}

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

    namespace
    {
      struct FinishedEventVisitor
      {
        using result_type = JobFinishedEvent;
        template<typename Visitor>
        result_type operator()(Visitor result) const
        {
          return  JobFinishedEvent ( _job_id, result);
        }
        const sdpa::job_id_t _job_id;
      };
    }

    LOAD_CONSTRUCT_DATA_DEF (JobFinishedEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (job_id);
      LOAD_FROM_ARCHIVE (sdpa::finished_reason_t, reason);

      FinishedEventVisitor visitor = {job_id};
      ::new (e) JobFinishedEvent(boost::apply_visitor(visitor, reason));
    }


  }
}
