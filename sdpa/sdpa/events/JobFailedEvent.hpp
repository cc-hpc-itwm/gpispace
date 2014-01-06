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
      typedef boost::shared_ptr<JobFailedEvent> Ptr;

      JobFailedEvent ( const address_t& a_from
                     , const address_t& a_to
                     , const sdpa::job_id_t& a_job_id
                     , int error_code = fhg::error::UNASSIGNED_ERROR
                     , std::string error_message = std::string()
                     )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
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

      int error_code() const
      {
        return m_error_code;
      }
      std::string const& error_message() const
      {
        return m_error_message;
      }

    private:
      int m_error_code;
      std::string m_error_message;
    };

    SAVE_CONSTRUCT_DATA_DEF (JobFailedEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      // \note Required, as Archive<< (int) takes an lvalue or const
      // rvalue and return value is not const, thus fails. (?!)
      const int weird_temporary (e->error_code());
      SAVE_TO_ARCHIVE (weird_temporary);
      SAVE_TO_ARCHIVE (e->error_message());
    }

    LOAD_CONSTRUCT_DATA_DEF (JobFailedEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (from, to, job_id);
      LOAD_FROM_ARCHIVE (int, error_code);
      LOAD_FROM_ARCHIVE (std::string, error_message);

      ::new (e) JobFailedEvent (from, to, job_id, error_code, error_message);
    }
  }
}

#endif
