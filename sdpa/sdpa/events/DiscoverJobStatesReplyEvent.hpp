#ifndef SDPA_DISCOVER_JOB_STATES_REPLY_EVENT_HPP
#define SDPA_DISCOVER_JOB_STATES_REPLY_EVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>
#include <sstream>

namespace sdpa
{
  namespace events
  {
    class DiscoverJobStatesReplyEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<DiscoverJobStatesReplyEvent> Ptr;

      DiscoverJobStatesReplyEvent ( const address_t& a_from
                                    , const address_t& a_to
                                    , const sdpa::job_id_t& a_job_id
                                    , const sdpa::job_id_t& discover_id
                                    , const sdpa::discovery_info_t& discover_result
                                    )
        : JobEvent (a_from, a_to, a_job_id)
        , discover_id_(discover_id)
        , discover_result_(discover_result)
      {}

      std::string str() const
      {
        return "DiscoverJobStatesReplyEvent";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleDiscoverJobStatesReplyEvent (this);
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
       SAVE_JOBEVENT_CONSTRUCT_DATA (e);
       SAVE_TO_ARCHIVE (e->discover_id());
       SAVE_TO_ARCHIVE (e->discover_result());
     }

     LOAD_CONSTRUCT_DATA_DEF (DiscoverJobStatesReplyEvent, e)
     {
       LOAD_JOBEVENT_CONSTRUCT_DATA (from, to, job_id);
       LOAD_FROM_ARCHIVE (sdpa::job_id_t, disc_id);
       LOAD_FROM_ARCHIVE (sdpa::discovery_info_t, disc_res);
       ::new (e) DiscoverJobStatesReplyEvent (from, to, job_id, disc_id, disc_res);
     }
  }
}

#endif
