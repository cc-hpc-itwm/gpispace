// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#if !GSPC_WITH_IML

#include <gspc/iml/macros.hpp>

static_assert ( gspc::WithIML_v<>
              , GSPC_WITHOUT_IML_ERROR_MESSAGE
              );

#else

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

#endif
