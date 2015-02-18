#pragma once

#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class SubscribeAckEvent : public MgmtEvent
    {
    public:
      typedef boost::shared_ptr<SubscribeAckEvent> Ptr;

      SubscribeAckEvent (const job_id_t& job_id)
        : MgmtEvent()
        , _job_id (job_id)
      { }

      const sdpa::job_id_t& job_id() const
      {
        return _job_id;
      }

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleSubscribeAckEvent (source, this);
      }

    private:
      sdpa::job_id_t _job_id;
    };

    SAVE_CONSTRUCT_DATA_DEF (SubscribeAckEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->job_id());
    }

    LOAD_CONSTRUCT_DATA_DEF (SubscribeAckEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (sdpa::job_id_t, job_id);

      ::new (e) SubscribeAckEvent (job_id);
    }
  }
}
