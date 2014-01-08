#ifndef SDPA_JOB_FINISHED_EVENT_HPP
#define SDPA_JOB_FINISHED_EVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa
{
  namespace events
  {
    class JobFinishedEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<JobFinishedEvent> Ptr;

      JobFinishedEvent ( const address_t& a_from
                       , const address_t& a_to
                       , const sdpa::job_id_t& a_job_id
                       , const job_result_t& job_result
                       )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
        , result_ (job_result)
      {}

      std::string str() const
      {
        return "JobFinishedEvent(" + job_id () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleJobFinishedEvent (this);
      }

      const job_result_t& result() const
      {
        return result_;
      }

    private:
      job_result_t result_;
    };

    SAVE_CONSTRUCT_DATA_DEF (JobFinishedEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->result());
    }

    LOAD_CONSTRUCT_DATA_DEF (JobFinishedEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (from, to, job_id);
      LOAD_FROM_ARCHIVE (job_result_t, result);

      ::new (e) JobFinishedEvent (from, to, job_id, result);
    }
  }
}

#endif
