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

#include <gspc/detail/dllexport.hpp>

#include <drts/drts.fwd.hpp>
#include <drts/pimpl.hpp>
#include <drts/virtual_memory.fwd.hpp>

#include <we/type/value.hpp>

#include <iml/SegmentAndAllocation.hpp>

#include <boost/filesystem/path.hpp>

#include <iostream>
#include <string>

namespace gspc
{
  namespace vmem
  {
    using beegfs_segment_description = iml::beegfs::SegmentDescription;
    using gaspi_segment_description = iml::gaspi::SegmentDescription;

    using segment_description = iml::SegmentDescription;
  }

  class GSPC_DLLEXPORT vmem_allocation
  {
  private:
    friend class scoped_runtime_system;

    iml::SegmentAndAllocation _alloc;

    vmem_allocation (iml::SegmentAndAllocation alloc);
    iml::SegmentAndAllocation const& iml_allocation() const
    {
      return _alloc;
    }

  public:
    std::size_t size() const;
    pnet::type::value::value_type global_memory_range() const;
    pnet::type::value::value_type global_memory_range ( std::size_t offset
                                                      , std::size_t size
                                                      ) const;

    vmem_allocation (vmem_allocation const&) = delete;
    vmem_allocation& operator= (vmem_allocation const&) = delete;

    vmem_allocation (vmem_allocation&&) = default;
    vmem_allocation& operator= (vmem_allocation&&) = delete;

    ~vmem_allocation() = default;
  };
}
