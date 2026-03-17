// Copyright (C) 2011,2013-2015,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/MgmtEvent.hpp>
#include <gspc/scheduler/types.hpp>


  namespace gspc::scheduler::events
  {
    class SubscribeEvent : public MgmtEvent
    {
    public:
      using Ptr = std::shared_ptr<SubscribeEvent>;

      SubscribeEvent (job_id_t const& job_id)
        : MgmtEvent()
        , _job_id (job_id)
      {}

      gspc::scheduler::job_id_t const& job_id() const
      {
        return _job_id;
      }

      void handleBy
        (gspc::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleSubscribeEvent (source, this);
      }

    private:
      gspc::scheduler::job_id_t _job_id;
    };

    SAVE_CONSTRUCT_DATA_DEF (SubscribeEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->job_id());
    }

    LOAD_CONSTRUCT_DATA_DEF (SubscribeEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (gspc::scheduler::job_id_t, job_id);

      ::new (e) SubscribeEvent (job_id);
    }
  }
