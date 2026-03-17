// Copyright (C) 2010,2013-2015,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/SchedulerEvent.hpp>


  namespace gspc::scheduler::events
  {
    class MgmtEvent : public gspc::scheduler::events::SchedulerEvent
    {
    public:
      using SchedulerEvent::SchedulerEvent;
    };
  }
