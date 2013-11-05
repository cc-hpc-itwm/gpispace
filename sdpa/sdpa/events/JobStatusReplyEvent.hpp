#ifndef SDPA_ReplyJobStatusEvent_HPP
#define SDPA_ReplyJobStatusEvent_HPP

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/types.hpp>

#include <fhg/error_codes.hpp>

namespace sdpa
{
  namespace events
  {
    class JobStatusReplyEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<JobStatusReplyEvent> Ptr;

      JobStatusReplyEvent()
        : JobEvent ("", "", "")
        , status_ ("UNKNOWN")
      {}

      JobStatusReplyEvent ( const address_t& a_from
                          , const address_t& a_to
                          , const sdpa::job_id_t& a_job_id
                          , const std::string &a_status = std::string()
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

      const std::string& status() const
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
      std::string status_;
      int m_error_code;
      std::string m_error_message;

      friend class boost::serialization::access;
      template <class Archive>
      void serialize (Archive & ar, unsigned int)
      {
        ar & boost::serialization::base_object<JobEvent> (*this);
        ar & status_;
        ar & m_error_code;
        ar & m_error_message;
      }
    };
  }
}

#endif
