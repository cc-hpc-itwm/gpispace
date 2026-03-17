// Copyright (C) 2010,2013-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/EventHandler.hpp>
#include <gspc/scheduler/events/JobEvent.hpp>

#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/net.hpp>


  namespace gspc::scheduler::events
  {
    class JobFinishedEvent : public JobEvent
    {
    public:
      using Ptr = std::shared_ptr<JobFinishedEvent>;

      JobFinishedEvent ( gspc::scheduler::job_id_t const& a_job_id
                       , gspc::we::type::Activity job_result
                       )
        : gspc::scheduler::events::JobEvent (a_job_id)
        , result_ (std::move (job_result))
      {}

      void handleBy
        (gspc::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleJobFinishedEvent (source, this);
      }

      gspc::we::type::Activity const& result() const
      {
        return result_;
      }

    private:
      gspc::we::type::Activity result_;
    };

    SAVE_CONSTRUCT_DATA_DEF (JobFinishedEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->result());
    }

    LOAD_CONSTRUCT_DATA_DEF (JobFinishedEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (job_id);
      LOAD_FROM_ARCHIVE (gspc::we::type::Activity, result);

      ::new (e) JobFinishedEvent (job_id, std::move (result));
    }
  }
