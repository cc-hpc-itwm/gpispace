// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/capability.hpp>
#include <sdpa/events/MgmtEvent.hpp>

#include <boost/optional.hpp>

namespace sdpa
{
  namespace events
  {
    class WorkerRegistrationEvent : public MgmtEvent
    {
    public:
      using Ptr = ::boost::shared_ptr<WorkerRegistrationEvent>;

      WorkerRegistrationEvent
        ( std::string const& name
        , Capabilities const& cpbset
        , unsigned long allocated_shared_memory_size
        , std::string const& hostname
        )
          : MgmtEvent()
          , _name (name)
          , cpbset_ (cpbset)
          , allocated_shared_memory_size_ (allocated_shared_memory_size)
          , hostname_(hostname)
      {}

      std::string const& name() const
      {
        return _name;
      }
      Capabilities const& capabilities() const
      {
        return cpbset_;
      }

      std::string const& hostname() const
      {
        return hostname_;
      }

      const unsigned long& allocated_shared_memory_size() const
      {
        return allocated_shared_memory_size_;
      }

      void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleWorkerRegistrationEvent (source, this);
      }

    private:
      std::string _name;
      Capabilities cpbset_;
      unsigned long allocated_shared_memory_size_;
      std::string hostname_;
    };

    SAVE_CONSTRUCT_DATA_DEF (WorkerRegistrationEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->name());
      SAVE_TO_ARCHIVE (e->capabilities());
      SAVE_TO_ARCHIVE (e->allocated_shared_memory_size());
      SAVE_TO_ARCHIVE (e->hostname());
    }

    LOAD_CONSTRUCT_DATA_DEF (WorkerRegistrationEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (std::string, name);
      LOAD_FROM_ARCHIVE (Capabilities, cpbset);
      LOAD_FROM_ARCHIVE (unsigned long, allocated_shared_memory_size);
      LOAD_FROM_ARCHIVE (std::string, hostname);

      ::new (e) WorkerRegistrationEvent ( name
                                        , cpbset
                                        , allocated_shared_memory_size
                                        , hostname
                                        );
    }
  }
}
