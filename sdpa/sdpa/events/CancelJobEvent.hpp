#ifndef SDPA_CANCELJOBEVENT_HPP
#define SDPA_CANCELJOBEVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/Serialization.hpp>

namespace sdpa
{
  namespace events
  {
    class CancelJobEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<CancelJobEvent> Ptr;

      CancelJobEvent ( const address_t& a_from
                     , const address_t& a_to
                     , const sdpa::job_id_t& a_job_id
                     , const std::string& a_reason
                     )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
        , _reason (a_reason)
      {}

      std::string str() const
      {
        return "CancelJobEvent(" + job_id ().str () + ")";
      }
      std::string const& reason() const
      {
        return _reason;
      }
      virtual void handleBy (EventHandler* handler)
      {
        handler->handleCancelJobEvent (this);
      }

    private:
      std::string _reason;
    };

    SAVE_CONSTRUCT_DATA_DEF (CancelJobEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->reason());
    }

    LOAD_CONSTRUCT_DATA_DEF (CancelJobEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (from, to, job_id);
      LOAD_FROM_ARCHIVE (std::string, reason);

      ::new (e) CancelJobEvent (from, to, job_id, reason);
    }
  }
}

#endif
