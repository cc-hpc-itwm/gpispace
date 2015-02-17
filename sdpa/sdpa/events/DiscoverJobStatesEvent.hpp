#pragma once

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class DiscoverJobStatesEvent : public sdpa::events::JobEvent
    {
    public:
      typedef boost::shared_ptr<DiscoverJobStatesEvent> Ptr;

      DiscoverJobStatesEvent ( const sdpa::job_id_t& a_job_id
                             , const sdpa::job_id_t& discover_id
                             )
        : sdpa::events::JobEvent (a_job_id)
        , discover_id_ (discover_id)
      {}

      const sdpa::job_id_t& discover_id() const
      {
        return discover_id_;
      }

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleDiscoverJobStatesEvent (source, this);
      }

    private:
      sdpa::job_id_t discover_id_;
    };

    SAVE_CONSTRUCT_DATA_DEF (DiscoverJobStatesEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->discover_id());
    }

    LOAD_CONSTRUCT_DATA_DEF (DiscoverJobStatesEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (job_id);
      LOAD_FROM_ARCHIVE (sdpa::job_id_t, disc_id);
      ::new (e) DiscoverJobStatesEvent (job_id, disc_id);
    }
  }
}
