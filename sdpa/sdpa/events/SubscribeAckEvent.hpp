#ifndef SDPA_SUBSCRIBE_ACK_EVENT_HPP
#define SDPA_SUBSCRIBE_ACK_EVENT_HPP 1

#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class SubscribeAckEvent : public MgmtEvent
    {
    public:
      typedef sdpa::shared_ptr<SubscribeAckEvent> Ptr;

      SubscribeAckEvent ( const address_t& a_from
                        , const address_t& a_to
                        , const job_id_list_t& listJobIds
                        )
        : MgmtEvent (a_from, a_to)
        , listJobIds_ (listJobIds)
      { }

      const sdpa::job_id_list_t& listJobIds() const
      {
        return listJobIds_;
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleSubscribeAckEvent (this);
      }

      std::string str() const
      {
        return "SubscribeAckEvent";
      }

    private:
      sdpa::job_id_list_t listJobIds_;
    };

    SAVE_CONSTRUCT_DATA_DEF (SubscribeAckEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->listJobIds());
    }

    LOAD_CONSTRUCT_DATA_DEF (SubscribeAckEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA (from, to);
      LOAD_FROM_ARCHIVE (sdpa::job_id_list_t, listJobIds);

      ::new (e) SubscribeAckEvent (from, to, listJobIds);
    }
  }
}

#endif
