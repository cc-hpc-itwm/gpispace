// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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
