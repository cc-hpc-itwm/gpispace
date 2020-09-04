// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <sdpa/discovery_info.hpp>
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
