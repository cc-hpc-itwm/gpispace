#pragma once

#include <sdpa/events/Serialization.hpp>
#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/types.hpp>

namespace sdpa
{
  namespace events
  {
    class ErrorEvent : public MgmtEvent
    {
    public:
      typedef boost::shared_ptr<ErrorEvent> Ptr;

      enum error_code_t
        {
          SDPA_EBACKLOGFULL,
          SDPA_ENODE_SHUTDOWN,
          SDPA_EUNKNOWN,
          SDPA_ENETWORKFAILURE
        };

      ErrorEvent
        ( const error_code_t &a_error_code
        , const std::string& a_reason
        //! \todo This should not be in _every_ ErrorEvent!
        , const boost::optional<sdpa::job_id_t>& jobId = boost::none
        )
          : MgmtEvent()
          , error_code_ (a_error_code)
          , reason_ (a_reason)
          , job_id_ (jobId)
      {}

      const std::string&reason() const
      {
        return reason_;
      }
      const error_code_t &error_code() const
      {
        return error_code_;
      }
      const boost::optional<sdpa::job_id_t>& job_id() const
      {
        return job_id_;
      }

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleErrorEvent (source, this);
      }

    private:
      error_code_t error_code_;
      std::string reason_;
      boost::optional<sdpa::job_id_t> job_id_;
    };

    SAVE_CONSTRUCT_DATA_DEF (ErrorEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->error_code());
      SAVE_TO_ARCHIVE (e->reason());
      SAVE_TO_ARCHIVE (e->job_id());
    }

    LOAD_CONSTRUCT_DATA_DEF (ErrorEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (ErrorEvent::error_code_t, error_code);
      LOAD_FROM_ARCHIVE (std::string, reason);
      LOAD_FROM_ARCHIVE (boost::optional<sdpa::job_id_t>, job_id);

      ::new (e) ErrorEvent (error_code, reason, job_id);
    }
  }
}
