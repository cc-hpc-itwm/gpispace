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
