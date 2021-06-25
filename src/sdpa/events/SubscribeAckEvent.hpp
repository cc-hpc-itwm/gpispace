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

#include <drts/client.fwd.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class SubscribeAckEvent : public MgmtEvent
    {
    public:
      typedef boost::shared_ptr<SubscribeAckEvent> Ptr;

      SubscribeAckEvent (const job_id_t& job_id)
        : MgmtEvent()
        , _job_id (job_id)
      { }

      const sdpa::job_id_t& job_id() const
      {
        return _job_id;
      }

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleSubscribeAckEvent (source, this);
      }

    private:
      sdpa::job_id_t _job_id;
    };

    SAVE_CONSTRUCT_DATA_DEF (SubscribeAckEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->job_id());
    }

    LOAD_CONSTRUCT_DATA_DEF (SubscribeAckEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (sdpa::job_id_t, job_id);

      ::new (e) SubscribeAckEvent (job_id);
    }
  }
}
