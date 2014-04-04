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

      SubscribeEvent ( const address_t& a_from
                     , const address_t& a_to
                     , const job_id_t& job_id
                     )
        : MgmtEvent (a_from, a_to)
        , _job_id (job_id)
      {}

      const sdpa::agent_id_t& subscriber() const
      {
        return from();
      }
      const sdpa::job_id_t& job_id() const
      {
        return _job_id;
      }

      virtual void handleBy (EventHandler* handler)
      {
    	handler->handleSubscribeEvent (this);
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
      LOAD_MGMTEVENT_CONSTRUCT_DATA (from, to);
      LOAD_FROM_ARCHIVE (sdpa::job_id_t, job_id);

      ::new (e) SubscribeEvent (from, to, job_id);
    }
  }
}

#endif
