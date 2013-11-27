#ifndef SDPA_WORKER_REGISTRATION_EVENT_HPP
#define SDPA_WORKER_REGISTRATION_EVENT_HPP 1

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/capability.hpp>

#include <boost/optional.hpp>

namespace sdpa
{
  namespace events
  {
    class WorkerRegistrationEvent : public MgmtEvent
    {
    public:
      typedef boost::shared_ptr<WorkerRegistrationEvent> Ptr;

      WorkerRegistrationEvent
        ( const address_t& a_from
        , const address_t& a_to
        , const boost::optional<unsigned int>& capacity = boost::none
        , const capabilities_set_t& cpbset = capabilities_set_t()
        , const unsigned int& agent_rank = 0
        , const sdpa::worker_id_t& agent_uuid = ""
        )
          : MgmtEvent (a_from, a_to)
          , capacity_ (capacity)
          , cpbset_ (cpbset)
          , rank_ (agent_rank)
          , agent_uuid_ (agent_uuid)
      {}

      std::string str() const
      {
        return "WorkerRegistrationEvent";
      }

      const boost::optional<unsigned int>& capacity() const
      {
        return capacity_;
      }
      const unsigned int& rank() const
      {
        return rank_;
      }
      const capabilities_set_t& capabilities() const
      {
        return cpbset_;
      }
      const sdpa::worker_id_t& agent_uuid() const
      {
        return agent_uuid_;
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleWorkerRegistrationEvent (this);
      }

    private:
      boost::optional<unsigned int> capacity_;
      capabilities_set_t cpbset_;
      unsigned int rank_;
      sdpa::worker_id_t agent_uuid_;
    };

    SAVE_CONSTRUCT_DATA_DEF (WorkerRegistrationEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->capacity());
      SAVE_TO_ARCHIVE (e->capabilities());
      SAVE_TO_ARCHIVE (e->rank());
      SAVE_TO_ARCHIVE (e->agent_uuid());
    }

    LOAD_CONSTRUCT_DATA_DEF (WorkerRegistrationEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA (from, to);
      LOAD_FROM_ARCHIVE (boost::optional<unsigned int>, capacity);
      LOAD_FROM_ARCHIVE (capabilities_set_t, cpbset);
      LOAD_FROM_ARCHIVE (unsigned int, agent_rank);
      LOAD_FROM_ARCHIVE (sdpa::worker_id_t, agent_uuid);

      ::new (e) WorkerRegistrationEvent
          (from, to, capacity, cpbset, agent_rank, agent_uuid);
    }
  }
}

#endif
