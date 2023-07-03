// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/types.hpp>

namespace sdpa
{
  namespace events
  {
    class SubscribeAckEvent : public MgmtEvent
    {
    public:
      using Ptr = ::boost::shared_ptr<SubscribeAckEvent>;

      SubscribeAckEvent (job_id_t const& job_id)
        : MgmtEvent()
        , _job_id (job_id)
      { }

      sdpa::job_id_t const& job_id() const
      {
        return _job_id;
      }

      void handleBy
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
