#ifndef SDPA_RESULTS_REPLY_EVENT_HPP
#define SDPA_RESULTS_REPLY_EVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa
{
  namespace events
  {
    class JobResultsReplyEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<JobResultsReplyEvent> Ptr;

      JobResultsReplyEvent ( const address_t& a_from
                           , const address_t& a_to
                           , const sdpa::job_id_t& a_job_id
                           , const job_result_t& a_result
                           )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
        , result_ (a_result)
      {}

      std::string str() const
      {
        return "JobResultsReplyEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleJobResultsReplyEvent (this);
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
      LOAD_JOBEVENT_CONSTRUCT_DATA (from, to, job_id);
      LOAD_FROM_ARCHIVE (job_result_t, result);

      ::new (e) JobResultsReplyEvent (from, to, job_id, result);
    }
  }
}

#endif
