#ifndef SDPA_SUBSCRIBE_EVENT_HPP
#define SDPA_SUBSCRIBE_EVENT_HPP 1

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/types.hpp>

namespace sdpa
{
  namespace events
  {
    class SubscribeEvent : public MgmtEvent
    {
    public:
      typedef boost::shared_ptr<SubscribeEvent> Ptr;

      SubscribeEvent (const job_id_t& job_id)
        : MgmtEvent()
        , _job_id (job_id)
      {}

      const sdpa::job_id_t& job_id() const
      {
        return _job_id;
      }

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleSubscribeEvent (source, this);
      }

    private:
      sdpa::job_id_t _job_id;
    };

    SAVE_CONSTRUCT_DATA_DEF (SubscribeEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->job_id());
    }

    LOAD_CONSTRUCT_DATA_DEF (SubscribeEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (sdpa::job_id_t, job_id);

      ::new (e) SubscribeEvent (job_id);
    }
  }
}

#endif
