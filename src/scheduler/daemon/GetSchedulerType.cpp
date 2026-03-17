// Copyright (C) 2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/drts/scheduler_types.hpp>

#include <gspc/drts/private/scheduler_types_implementation.hpp>

#include <gspc/scheduler/daemon/GetSchedulerType.hpp>

#include <gspc/we/type/Activity.hpp>


  namespace gspc::scheduler::daemon
  {
    gspc::scheduler::Type get_scheduler_type (gspc::we::type::Activity const& activity)
    {
      using namespace gspc::scheduler;

      if (activity.might_have_tasks_requiring_multiple_workers())
      {
        return CostAwareWithWorkStealing
          (CostAwareWithWorkStealing::CoallocationWithBackfilling());
      }

      if ( activity.might_use_virtual_memory()
         || activity.might_use_modules_with_multiple_implementations()
         )
      {
        return CostAwareWithWorkStealing
          (CostAwareWithWorkStealing::SingleAllocation());
      }

      return GreedyWithWorkStealing{};
    }
  }
