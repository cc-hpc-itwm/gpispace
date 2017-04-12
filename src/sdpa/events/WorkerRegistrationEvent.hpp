#pragma once

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/capability.hpp>
#include <sdpa/types.hpp>

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
        , boost::optional<intertwine::vmem::cache_id_t> cache_id
        , boost::optional<intertwine::vmem::size_t> vmem_cache_size_
        , boost::optional<intertwine::vmem::rank_t> vmem_rank_
        , bool children_allowed
        )
          : MgmtEvent()
          , _name (name)
          , cpbset_ (cpbset)
          , vmem_cache_id (cache_id)
          , vmem_cache_size (vmem_cache_size_)
          , vmem_rank (vmem_rank_)
          , children_allowed_(children_allowed)
      {}

      std::string const& name() const
      {
        return _name;
      }
      const capabilities_set_t& capabilities() const
      {
        return cpbset_;
      }

      const bool& children_allowed() const
      {
	return children_allowed_;
      }

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleWorkerRegistrationEvent (source, this);
      }

    private:
      std::string _name;
      capabilities_set_t cpbset_;
    public:
      boost::optional<intertwine::vmem::cache_id_t> vmem_cache_id;
      boost::optional<intertwine::vmem::size_t> vmem_cache_size;
      boost::optional<intertwine::vmem::rank_t> vmem_rank;
    private:
      bool children_allowed_;
    };

    SAVE_CONSTRUCT_DATA_DEF (WorkerRegistrationEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->name());
      SAVE_TO_ARCHIVE (e->capabilities());
      SAVE_TO_ARCHIVE (e->vmem_cache_id);
      SAVE_TO_ARCHIVE (e->vmem_cache_size);
      SAVE_TO_ARCHIVE (e->vmem_rank);
      SAVE_TO_ARCHIVE (e->children_allowed());
    }

    LOAD_CONSTRUCT_DATA_DEF (WorkerRegistrationEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (std::string, name);
      LOAD_FROM_ARCHIVE (capabilities_set_t, cpbset);
      LOAD_FROM_ARCHIVE (boost::optional<intertwine::vmem::cache_id_t>, vmem_cache_id);
      LOAD_FROM_ARCHIVE (boost::optional<intertwine::vmem::size_t>, vmem_cache_size);
      LOAD_FROM_ARCHIVE (boost::optional<intertwine::vmem::rank_t>, vmem_rank);
      LOAD_FROM_ARCHIVE (bool, children_allowed);

      ::new (e) WorkerRegistrationEvent ( name
                                        , cpbset
                                        , vmem_cache_id
                                        , vmem_cache_size
                                        , vmem_rank
                                        , children_allowed
                                        );
    }
  }
}
