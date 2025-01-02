// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <drts/scheduler_types.hpp>

#include <drts/private/scheduler_types_implementation.hpp>

#include <sdpa/daemon/GetSchedulerType.hpp>

#include <we/type/Activity.hpp>

namespace sdpa
{
  namespace daemon
  {
    gspc::scheduler::Type get_scheduler_type (we::type::Activity const& activity)
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
}
