#ifndef SDPA_ERROREVENT_HPP
#define SDPA_ERROREVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/types.hpp>

namespace sdpa
{
  namespace events
  {
    class ErrorEvent : public MgmtEvent
    {
    public:
      typedef sdpa::shared_ptr<ErrorEvent> Ptr;

      enum error_code_t
        {
          SDPA_ENOERROR = 0,
          SDPA_ENOJOBAVAIL,
          SDPA_EJOBNOTFOUND,
          SDPA_EJOBEXISTS,
          SDPA_EJOBREJECTED,
          SDPA_EJOBNOTDELETED,
          SDPA_EWORKERNOTREG,
          SDPA_ENODE_SHUTDOWN,
          SDPA_EBUSY,
          SDPA_EAGAIN,
          SDPA_EUNKNOWN,
          SDPA_EPERM,
          SDPA_ENETWORKFAILURE
        };

      ErrorEvent()
        : MgmtEvent()
        , error_code_ (SDPA_EUNKNOWN)
        , reason_ ("unknown reason")
      {}

      ErrorEvent
        ( const address_t &a_from
        , const address_t &a_to
        , const error_code_t &a_error_code
        , const std::string& a_reason
        , const sdpa::job_id_t& jobId = sdpa::job_id_t::invalid_job_id()
        )
          : MgmtEvent (a_from, a_to)
          , error_code_ (a_error_code)
          , reason_ (a_reason)
          , job_id_ (jobId)
      {}

      const std::string&reason() const
      {
        return reason_;
      }
      std::string &reason()
      {
        return reason_;
      }
      const error_code_t &error_code() const
      {
        return error_code_;
      }
      error_code_t& error_code()
      {
        return error_code_;
      }
      const sdpa::job_id_t& job_id() const
      {
        return job_id_;
      }
      sdpa::job_id_t& job_id()
      {
        return job_id_;
      }

      std::string str() const
      {
        return "ErrorEvent";
      }

      int priority() const
      {
        return 1;
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleErrorEvent (this);
      }

    private:
      error_code_t error_code_;
      std::string reason_;
      sdpa::job_id_t job_id_;
    };
  }
}

#endif
