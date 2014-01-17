#ifndef SDPA_CANCELJOBACKEVENT_HPP
#define SDPA_CANCELJOBACKEVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/Serialization.hpp>

namespace sdpa
{
  namespace events
  {
    class CancelJobAckEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<CancelJobAckEvent> Ptr;

      CancelJobAckEvent ( const address_t& a_from
                        , const address_t& a_to
                        , const sdpa::job_id_t& a_job_id
                        )
        :  sdpa::events::JobEvent (a_from, a_to, a_job_id)
      {}

      std::string str() const
      {
        return "CancelJobAckEvent(" + job_id () + ")";
      }
      virtual void handleBy (EventHandler* handler)
      {
        handler->handleCancelJobAckEvent (this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (CancelJobAckEvent)
  }
}

#endif
