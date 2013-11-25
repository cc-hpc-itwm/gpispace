#ifndef SDPA_JOB_FAILED_EVENT_HPP
#define SDPA_JOB_FAILED_EVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>
#include <fhg/error_codes.hpp>

namespace sdpa
{
  namespace events
  {
    class JobFailedEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<JobFailedEvent> Ptr;

      JobFailedEvent ( const address_t& a_from
                     , const address_t& a_to
                     , const sdpa::job_id_t& a_job_id
                     , const job_result_t &job_result
                     , int error_code = fhg::error::UNASSIGNED_ERROR
                     , std::string error_message = std::string()
                     )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
        , result_ (job_result)
        , m_error_code (error_code)
        , m_error_message (error_message)
      {}

      std::string str() const
      {
        return "JobFailedEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleJobFailedEvent (this);
      }

      const job_result_t& result() const
      {
        return result_;
      }
      int error_code() const
      {
        return m_error_code;
      }
      std::string const& error_message() const
      {
        return m_error_message;
      }

    private:
      job_result_t result_;
      int m_error_code;
      std::string m_error_message;
    };

    SAVE_CONSTRUCT_DATA_DEF (JobFailedEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->result());
      // \note Required, as Archive<< (int) takes an lvalue or const
      // rvalue and return value is not const, thus fails. (?!)
      const int weird_temporary (e->error_code());
      SAVE_TO_ARCHIVE (weird_temporary);
      SAVE_TO_ARCHIVE (e->error_message());
    }

    LOAD_CONSTRUCT_DATA_DEF (JobFailedEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (from, to, job_id);
      LOAD_FROM_ARCHIVE (job_result_t, result);
      LOAD_FROM_ARCHIVE (int, error_code);
      LOAD_FROM_ARCHIVE (std::string, error_message);

      ::new (e) JobFailedEvent
          (from, to, job_id, result, error_code, error_message);
    }
  }
}

#endif
