// Copyright (C) 2010,2012-2016,2019-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/EventHandler.hpp>
#include <gspc/scheduler/events/SchedulerEvent.hpp>
#include <gspc/scheduler/types.hpp>

#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/net.hpp>


  namespace gspc::scheduler::events
  {
    class SubmitJobEvent : public SchedulerEvent
    {
    public:
      using Ptr = std::shared_ptr<SubmitJobEvent>;

      SubmitJobEvent
        ( std::optional<gspc::scheduler::job_id_t> const& a_job_id
        , gspc::we::type::Activity activity
        , std::optional<std::string> const& implementation
        , std::set<worker_id_t> const& workers = {}
        )
          : SchedulerEvent()
          , _job_id (a_job_id)
          , _activity (std::move (activity))
          , _implementation (implementation)
          , _workers (workers)
      {}

      std::optional<gspc::scheduler::job_id_t> const& job_id() const
      {
        return _job_id;
      }
      gspc::we::type::Activity const& activity() const
      {
        return _activity;
      }
      std::optional<std::string> const& implementation() const
      {
        return _implementation;
      }
      std::set<worker_id_t> const& workers() const
      {
        return _workers;
      }

      void handleBy
        (gspc::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleSubmitJobEvent (source, this);
      }

    private:
      std::optional<gspc::scheduler::job_id_t> _job_id;
      gspc::we::type::Activity _activity;
      std::optional<std::string> _implementation;
      std::set<worker_id_t> _workers;
    };

    SAVE_CONSTRUCT_DATA_DEF (SubmitJobEvent, e)
    {
      SAVE_SCHEDULEREVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->job_id());
      SAVE_TO_ARCHIVE (e->activity());
      SAVE_TO_ARCHIVE (e->implementation());
      SAVE_TO_ARCHIVE (e->workers());
    }

    LOAD_CONSTRUCT_DATA_DEF (SubmitJobEvent, e)
    {
      LOAD_SCHEDULEREVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (std::optional<gspc::scheduler::job_id_t>, job_id);
      LOAD_FROM_ARCHIVE (gspc::we::type::Activity, activity);
      LOAD_FROM_ARCHIVE (std::optional<std::string>, implementation);
      LOAD_FROM_ARCHIVE (std::set<gspc::scheduler::worker_id_t>, workers);

      ::new (e) SubmitJobEvent (job_id, std::move (activity), implementation, workers);
    }
  }
