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

#include <drts/scheduler_types.hpp>

#include <boost/variant.hpp>

#include <string>

namespace gspc
{
  namespace scheduler
  {
    struct CostAwareWithWorkStealing::Implementation
    {
      using Type = ::boost::variant
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
