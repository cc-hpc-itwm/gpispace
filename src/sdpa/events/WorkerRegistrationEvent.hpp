#pragma once

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/capability.hpp>

#include <drts/resource/ID.hpp>
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
        ( std::string const& name
        , const capabilities_set_t& cpbset
        , const unsigned long allocated_shared_memory_size
        , bool children_allowed
        , const std::string& hostname
        , std::vector<gspc::resource::ID> const& resource_ids
        )
          : MgmtEvent()
          , _name (name)
          , cpbset_ (cpbset)
          , allocated_shared_memory_size_ (allocated_shared_memory_size)
          , children_allowed_(children_allowed)
          , hostname_(hostname)
          , _resource_ids (resource_ids)
      {}

      std::string const& name() const
      {
        return _name;
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

      const unsigned long& allocated_shared_memory_size() const
      {
        return allocated_shared_memory_size_;
      }

      std::vector<gspc::resource::ID> const& resource_ids() const
      {
        return _resource_ids;
      }

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleWorkerRegistrationEvent (source, this);
      }

    private:
      std::string _name;
      capabilities_set_t cpbset_;
      unsigned long allocated_shared_memory_size_;
      bool children_allowed_;
      std::string hostname_;
      std::vector<gspc::resource::ID> _resource_ids;
    };

    SAVE_CONSTRUCT_DATA_DEF (WorkerRegistrationEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->name());
      SAVE_TO_ARCHIVE (e->capabilities());
      SAVE_TO_ARCHIVE (e->allocated_shared_memory_size());
      SAVE_TO_ARCHIVE (e->children_allowed());
      SAVE_TO_ARCHIVE (e->hostname());
      SAVE_TO_ARCHIVE (e->resource_ids());
    }

    LOAD_CONSTRUCT_DATA_DEF (WorkerRegistrationEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (std::string, name);
      LOAD_FROM_ARCHIVE (capabilities_set_t, cpbset);
      LOAD_FROM_ARCHIVE (unsigned long, allocated_shared_memory_size);
      LOAD_FROM_ARCHIVE (bool, children_allowed);
      LOAD_FROM_ARCHIVE (std::string, hostname);
      LOAD_FROM_ARCHIVE (std::vector<gspc::resource::ID>, resource_ids);

      ::new (e) WorkerRegistrationEvent ( name
                                        , cpbset
                                        , allocated_shared_memory_size
                                        , children_allowed
                                        , hostname
                                        , resource_ids
                                        );
    }
  }
}
