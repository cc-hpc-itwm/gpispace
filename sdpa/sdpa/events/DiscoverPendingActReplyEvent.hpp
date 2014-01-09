#ifndef SDPA_DISCOVER_PENDING_ACT_REPLY_EVENT_HPP
#define SDPA_DISCOVER_PENDING_ACT_REPLY_EVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class DiscoverPendingActReplyEvent : public sdpa::events::JobEvent
    {
    public:
      typedef boost::shared_ptr<JobEvent> Ptr;

      DiscoverPendingActReplyEvent ( const address_t& a_from
                                           , const address_t& a_to
                                           , const sdpa::job_id_t& a_job_id
                                           )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
      {}

      std::string str() const
      {
        return "DiscoverPendingActReplyEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        //handler->handleDiscoverPendingActReplyEvent (this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (DiscoverPendingActReplyEvent)
  }
}

#endif
