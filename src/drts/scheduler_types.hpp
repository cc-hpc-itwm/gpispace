// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <variant>

namespace gspc
{
  namespace scheduler
  {
    struct CostAwareWithWorkStealing
    {
      struct SingleAllocation{};
      struct CoallocationWithBackfilling{};

      CostAwareWithWorkStealing() = delete;
      CostAwareWithWorkStealing (CostAwareWithWorkStealing const&) = delete;
      CostAwareWithWorkStealing (CostAwareWithWorkStealing&&) noexcept;
      CostAwareWithWorkStealing& operator= (CostAwareWithWorkStealing const&) = delete;
      CostAwareWithWorkStealing& operator= (CostAwareWithWorkStealing&&) noexcept;

      CostAwareWithWorkStealing (SingleAllocation);
      CostAwareWithWorkStealing (CoallocationWithBackfilling);

      ~CostAwareWithWorkStealing();

      struct Implementation;
      std::unique_ptr<Implementation> _;
    };

    struct GreedyWithWorkStealing
    {
      GreedyWithWorkStealing() = default;
      GreedyWithWorkStealing (GreedyWithWorkStealing const&) = delete;
      GreedyWithWorkStealing (GreedyWithWorkStealing&&) = default;
      GreedyWithWorkStealing& operator= (GreedyWithWorkStealing const&) = delete;
      GreedyWithWorkStealing& operator= (GreedyWithWorkStealing&&) = default;
      ~GreedyWithWorkStealing() = default;
    };

    using Type = std::variant<GreedyWithWorkStealing, CostAwareWithWorkStealing>;
  }
}
