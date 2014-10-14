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
        , const boost::optional<unsigned int>& capacity
        , const capabilities_set_t& cpbset
        , bool children_allowed
        , const std::string& hostname
        )
          : MgmtEvent (a_from, a_to)
          , capacity_ (capacity)
          , cpbset_ (cpbset)
          , children_allowed_(children_allowed)
          , hostname_(hostname)
      {}

      const boost::optional<unsigned int>& capacity() const
      {
        return capacity_;
      }
      const capabilities_set_t& capabilities() const
      {
        return cpbset_;
      }

      const std::string& hostname() const
      {
        return hostname_;
      }

      const bool& children_allowed() const
      {
	return children_allowed_;
      }

      virtual void handleBy (EventHandler* handler) override
      {
        handler->handleWorkerRegistrationEvent (this);
      }

    private:
      boost::optional<unsigned int> capacity_;
      capabilities_set_t cpbset_;
      bool children_allowed_;
      std::string hostname_;
    };

    SAVE_CONSTRUCT_DATA_DEF (WorkerRegistrationEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->capacity());
      SAVE_TO_ARCHIVE (e->capabilities());
      SAVE_TO_ARCHIVE (e->children_allowed());
      SAVE_TO_ARCHIVE (e->hostname());
    }

    LOAD_CONSTRUCT_DATA_DEF (WorkerRegistrationEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA (from, to);
      LOAD_FROM_ARCHIVE (boost::optional<unsigned int>, capacity);
      LOAD_FROM_ARCHIVE (capabilities_set_t, cpbset);
      LOAD_FROM_ARCHIVE (bool, children_allowed);
      LOAD_FROM_ARCHIVE (std::string, hostname);

      ::new (e) WorkerRegistrationEvent (from, to, capacity, cpbset, children_allowed, hostname);
    }
  }
}

#endif
