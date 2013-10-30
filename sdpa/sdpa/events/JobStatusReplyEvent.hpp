#ifndef SDPA_ReplyJobStatusEvent_HPP
#define SDPA_ReplyJobStatusEvent_HPP

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/types.hpp>

namespace sdpa { namespace events {
  class JobStatusReplyEvent : public JobEvent
  {
    public:
      typedef sdpa::shared_ptr<JobStatusReplyEvent> Ptr;
      typedef sdpa::status_t status_t;

      JobStatusReplyEvent()
        : JobEvent("", "", "")
        , status_("UNKNOWN")
      {}

      JobStatusReplyEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id, const status_t &a_status = status_t())
        : sdpa::events::JobEvent(a_from, a_to, a_job_id)
        , status_(a_status)
      { }

      std::string str() const
      {
        return "JobStatusReplyEvent(" + job_id ().str () + ")";
      }

      const status_t &status() const { return status_; }
      status_t &status() { return status_; }

      int error_code () const { return m_error_code; }
      int & error_code () { return m_error_code; }
      std::string const & error_message() const { return m_error_message; }
      std::string & error_message() { return m_error_message; }

      virtual void handleBy(EventHandler *handler)
      {
        handler->handleJobStatusReplyEvent(this);
      }
    private:
      status_t status_;
      int m_error_code;
      std::string m_error_message;
  };
}}

#endif
