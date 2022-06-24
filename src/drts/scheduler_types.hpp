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

#pragma once

#include <boost/variant.hpp>

#include <memory>

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

    using Type = ::boost::variant<GreedyWithWorkStealing, CostAwareWithWorkStealing>;
  }
}
