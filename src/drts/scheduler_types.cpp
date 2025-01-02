// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
      (CostAwareWithWorkStealing&&) noexcept = default;
    CostAwareWithWorkStealing&
      CostAwareWithWorkStealing::operator= (CostAwareWithWorkStealing&&) noexcept = default;
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
      return fhg::util::visit
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
      return fhg::util::visit
        ( scheduler_type
        , [] (auto const& type) { return to_string (type); }
        );
    }
  }
}
