// Copyright (C) 2010,2013-2015,2021-2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/SchedulerEvent.hpp>
#include <gspc/scheduler/types.hpp>

#include <string>


  namespace gspc::scheduler::events
  {
    class JobEvent : public gspc::scheduler::events::SchedulerEvent
    {
    public:
      JobEvent (gspc::scheduler::job_id_t const& a_job_id)
        : SchedulerEvent()
        , job_id_ (a_job_id)
      {}

      gspc::scheduler::job_id_t const& job_id() const
      {
        return job_id_;
      }

    private:
      gspc::scheduler::job_id_t job_id_;
    };
  }
