// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

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
        ( std::string const& name
        , capabilities_set_t const& cpbset
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
      capabilities_set_t const& capabilities() const
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

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleWorkerRegistrationEvent (source, this);
      }

    private:
      std::string _name;
      capabilities_set_t cpbset_;
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
      LOAD_FROM_ARCHIVE (capabilities_set_t, cpbset);
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
