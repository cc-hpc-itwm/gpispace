#pragma once

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa
{
  namespace events
  {
    class JobResultsReplyEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<JobResultsReplyEvent> Ptr;

      JobResultsReplyEvent ( const sdpa::job_id_t& a_job_id
                           , const job_result_t& a_result
                           )
        : sdpa::events::JobEvent (a_job_id)
        , result_ (a_result)
      {}

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleJobResultsReplyEvent (source, this);
      }

      const job_result_t& result() const
      {
        return result_;
      }

    private:
      job_result_t result_;
    };

    SAVE_CONSTRUCT_DATA_DEF (JobResultsReplyEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->result());
    }

    LOAD_CONSTRUCT_DATA_DEF (JobResultsReplyEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (job_id);
      LOAD_FROM_ARCHIVE (job_result_t, result);

      ::new (e) JobResultsReplyEvent (job_id, result);
    }
  }
}
