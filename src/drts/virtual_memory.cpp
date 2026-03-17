// Copyright (C) 2014-2015,2018,2020-2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/drts/virtual_memory.hpp>

#include <gspc/drts/drts_iml.hpp>
#include <gspc/we/type/value/to_value.hpp>

#include <fmt/core.h>
#include <cstddef>
#include <stdexcept>
#include <utility>

namespace gspc
{
  vmem_allocation::vmem_allocation (gspc::iml::SegmentAndAllocation alloc)
    : _alloc (std::move (alloc))
  {}
  std::size_t vmem_allocation::size() const
  {
    return _alloc.size();
  }

  gspc::pnet::type::value::value_type vmem_allocation::global_memory_range
    ( std::size_t const offset
    , std::size_t const size
    ) const
  {
    if ((offset + size) > _alloc.size())
    {
      throw std::logic_error
        { fmt::format
            ( "slice [{}, {}) is outside of allocation"
            , offset
            , offset + size
            )
        };
    }

    return gspc::pnet::type::value::to_value
      (_alloc.memory_region (offset, size));
  }
  gspc::pnet::type::value::value_type vmem_allocation::global_memory_range() const
  {
    return global_memory_range (0UL, _alloc.size());
  }
}
