#pragma once

#include <sdpa/events/MgmtEvent.hpp>
#include <sstream>

namespace sdpa
{
  namespace events
  {
    class DiscoverJobStatesReplyEvent : public MgmtEvent
    {
    public:
      typedef boost::shared_ptr<DiscoverJobStatesReplyEvent> Ptr;

      DiscoverJobStatesReplyEvent ( const sdpa::job_id_t& discover_id
                                  , const sdpa::discovery_info_t& discover_result
                                  )
        : MgmtEvent()
        , discover_id_ (discover_id)
        , discover_result_ (discover_result)
      {}

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleDiscoverJobStatesReplyEvent (source, this);
      }

      const sdpa::job_id_t& discover_id() const
      {
        return discover_id_;
      }
      const sdpa::discovery_info_t& discover_result() const
      {
        return discover_result_;
      }

    private:
      sdpa::job_id_t discover_id_;
      sdpa::discovery_info_t discover_result_;
    };

    SAVE_CONSTRUCT_DATA_DEF (DiscoverJobStatesReplyEvent, e)
     {
       SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
       SAVE_TO_ARCHIVE (e->discover_id());
       SAVE_TO_ARCHIVE (e->discover_result());
     }

     LOAD_CONSTRUCT_DATA_DEF (DiscoverJobStatesReplyEvent, e)
     {
       LOAD_MGMTEVENT_CONSTRUCT_DATA();
       LOAD_FROM_ARCHIVE (sdpa::job_id_t, disc_id);
       LOAD_FROM_ARCHIVE (sdpa::discovery_info_t, disc_res);
       ::new (e) DiscoverJobStatesReplyEvent (disc_id, disc_res);
     }
  }
}
