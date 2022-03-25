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

#include <drts/private/scheduler_types_implementation.hpp>
#include <drts/scheduler_types.hpp>

#include <util-generic/functor_visitor.hpp>

#include <memory>
#include <string>

namespace gspc
{
  namespace scheduler
  {
    CostAwareWithWorkStealing::CostAwareWithWorkStealing
        (CostAwareWithWorkStealing::SingleAllocation scheduler)
      : _ (std::make_unique<Implementation> (scheduler))
    {}

    CostAwareWithWorkStealing::CostAwareWithWorkStealing
        (CostAwareWithWorkStealing::CoallocationWithBackfilling scheduler)
      : _ (std::make_unique<Implementation> (scheduler))
    {}

    CostAwareWithWorkStealing::Implementation::Implementation
        (CostAwareWithWorkStealing::SingleAllocation scheduler)
      : _constructed_from (scheduler)
    {}

    CostAwareWithWorkStealing::Implementation::Implementation
        (CostAwareWithWorkStealing::CoallocationWithBackfilling scheduler)
      : _constructed_from (scheduler)
    {}

    CostAwareWithWorkStealing::CostAwareWithWorkStealing
      (CostAwareWithWorkStealing&&) = default;
    CostAwareWithWorkStealing&
      CostAwareWithWorkStealing::operator= (CostAwareWithWorkStealing&&) = default;
    CostAwareWithWorkStealing::~CostAwareWithWorkStealing() = default;
    CostAwareWithWorkStealing::Implementation::~Implementation() = default;

    std::string to_string (CostAwareWithWorkStealing::SingleAllocation const&)
    {
      return "CostAwareWithWorkStealing::SingleAllocation";
    }

    CostAwareWithWorkStealing::Implementation::Type
      CostAwareWithWorkStealing::Implementation::constructed_from() const
    {
      return _constructed_from;
    }

    std::string to_string
      (CostAwareWithWorkStealing::CoallocationWithBackfilling const&)
    {
      return "CostAwareWithWorkStealing::CoallocationWithBackfilling";
    }

    std::string to_string
      (CostAwareWithWorkStealing::Implementation const& impl)
    {
      return fhg::util::visit<std::string>
        ( impl._constructed_from
        , [] (auto const& type) { return to_string (type); }
        );
    }

    std::string to_string
      (CostAwareWithWorkStealing const& scheduler)
    {
      return to_string (*scheduler._);
    }

    std::string to_string (GreedyWithWorkStealing const&)
    {
      return "GreedyWithWorkStealing";
    }

    std::string to_string
      (Type const& scheduler_type)
    {
      return fhg::util::visit<std::string>
        ( scheduler_type
        , [] (auto const& type) { return to_string (type); }
        );
    }
  }
}
