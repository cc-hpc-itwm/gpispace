#ifndef SDPA_DISCOVER_JOB_STATES_EVENT_HPP
#define SDPA_DISCOVER_JOB_STATES_EVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class DiscoverJobStatesEvent : public sdpa::events::JobEvent
    {
    public:
      typedef boost::shared_ptr<DiscoverJobStatesEvent> Ptr;

      DiscoverJobStatesEvent ( const address_t& a_from
                              , const address_t& a_to
                              , const sdpa::job_id_t& a_job_id
                              , const std::string& discover_id
                              )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
        , discover_id_(discover_id)
      {}

      std::string str() const
      {
        return "DiscoverJobStatestEvent(" + job_id () + ")";
      }

      const std::string& discover_id() const { return discover_id_; }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleDiscoverJobStatesEvent (this);
      }

    private:
      std::string discover_id_;
    };

    SAVE_CONSTRUCT_DATA_DEF (DiscoverJobStatesEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->discover_id());
    }

    LOAD_CONSTRUCT_DATA_DEF (DiscoverJobStatesEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (from, to, job_id);
      LOAD_FROM_ARCHIVE ( std::string, disc_id);
      ::new (e) DiscoverJobStatesEvent (from, to, job_id, disc_id);
    }
  }
}

#endif