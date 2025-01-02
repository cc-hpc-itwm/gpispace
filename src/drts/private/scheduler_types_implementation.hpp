// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <drts/scheduler_types.hpp>

#include <string>
#include <variant>

namespace gspc
{
  namespace scheduler
  {
    struct CostAwareWithWorkStealing::Implementation
    {
      using Type = std::variant
                     < CostAwareWithWorkStealing::SingleAllocation
                     , CostAwareWithWorkStealing::CoallocationWithBackfilling
                     >;

      Implementation (CostAwareWithWorkStealing::SingleAllocation);
      Implementation (CostAwareWithWorkStealing::CoallocationWithBackfilling);
      ~Implementation();
      Implementation (Implementation const&) = delete;
      Implementation (Implementation&&) = delete;
      Implementation& operator= (Implementation const&) = delete;
      Implementation& operator= (Implementation&&) = delete;

      friend std::string to_string (Implementation const&);

      Type constructed_from() const;
    private:
      Type _constructed_from;
    };

    std::string to_string (CostAwareWithWorkStealing::SingleAllocation const&);
    std::string to_string (CostAwareWithWorkStealing::CoallocationWithBackfilling const&);
    std::string to_string (CostAwareWithWorkStealing::Implementation const&);
    std::string to_string (CostAwareWithWorkStealing const&);
    std::string to_string (GreedyWithWorkStealing const&);
    std::string to_string (Type const&);
  }
}
