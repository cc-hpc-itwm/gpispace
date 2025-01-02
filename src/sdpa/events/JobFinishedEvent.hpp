// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/events/EventHandler.hpp>
#include <sdpa/events/JobEvent.hpp>

#include <we/type/Activity.hpp>
#include <we/type/net.hpp>

namespace sdpa
{
  namespace events
  {
    class JobFinishedEvent : public JobEvent
    {
    public:
      using Ptr = ::boost::shared_ptr<JobFinishedEvent>;

      JobFinishedEvent ( sdpa::job_id_t const& a_job_id
                       , we::type::Activity job_result
                       )
        : sdpa::events::JobEvent (a_job_id)
        , result_ (std::move (job_result))
      {}

      void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleJobFinishedEvent (source, this);
      }

      we::type::Activity const& result() const
      {
        return result_;
      }

    private:
      we::type::Activity result_;
    };

    SAVE_CONSTRUCT_DATA_DEF (JobFinishedEvent, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->result());
    }

    LOAD_CONSTRUCT_DATA_DEF (JobFinishedEvent, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (job_id);
      LOAD_FROM_ARCHIVE (we::type::Activity, result);

      ::new (e) JobFinishedEvent (job_id, std::move (result));
    }
  }
}
