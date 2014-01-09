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
                     )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
      {}

      std::string str() const
      {
        return "CancelJobEvent(" + job_id () + ")";
      }
      virtual void handleBy (EventHandler* handler)
      {
        handler->handleCancelJobEvent (this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (CancelJobEvent)
  }
}

#endif
