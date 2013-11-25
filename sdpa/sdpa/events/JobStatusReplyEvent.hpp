#ifndef SDPA_ReplyJobStatusEvent_HPP
#define SDPA_ReplyJobStatusEvent_HPP

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/types.hpp>
#include <sdpa/job_states.hpp>

#include <fhg/error_codes.hpp>

namespace sdpa
{
  namespace events
  {
    class JobStatusReplyEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<JobStatusReplyEvent> Ptr;

      JobStatusReplyEvent ( const address_t& a_from
                          , const address_t& a_to
                          , const sdpa::job_id_t& a_job_id
                          , const sdpa::status::code& a_status
                          , int const error_code = fhg::error::UNASSIGNED_ERROR
                          , std::string const& error_message = std::string()
                          )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
        , status_ (a_status)
        , m_error_code (error_code)
        , m_error_message (error_message)
      { }

      std::string str() const
      {
        return "JobStatusReplyEvent(" + job_id().str() + ")";
      }

      sdpa::status::code status() const
      {
        return status_;
      }
      int error_code() const
      {
        return m_error_code;
      }
      std::string const& error_message() const
      {
        return m_error_message;
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleJobStatusReplyEvent (this);
      }

    private:
      sdpa::status::code status_;
      int m_error_code;
      std::string m_error_message;
    };

    SAVE_CONSTRUCT_DATA_DEF (JobStatusReplyEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE_WITH_TEMPORARY (sdpa::status::code, e->status());
      SAVE_TO_ARCHIVE_WITH_TEMPORARY (int, e->error_code());
      SAVE_TO_ARCHIVE (e->error_message());
    }

    LOAD_CONSTRUCT_DATA_DEF (JobStatusReplyEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (from, to, job_id);
      LOAD_FROM_ARCHIVE (sdpa::status::code, status);
      LOAD_FROM_ARCHIVE (int, error_code);
      LOAD_FROM_ARCHIVE (std::string, error_message);

      ::new (e) JobStatusReplyEvent
          (from, to, job_id, status, error_code, error_message);
    }
  }
}

#endif
