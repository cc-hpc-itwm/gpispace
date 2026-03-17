// Copyright (C) 2014-2016,2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#if !GSPC_WITH_IML

#include <gspc/iml/macros.hpp>

static_assert ( gspc::WithIML_v<>
              , GSPC_WITHOUT_IML_ERROR_MESSAGE
              );

#else

#include <gspc/detail/export.hpp>

#include <gspc/drts/drts.fwd.hpp>
#include <gspc/drts/pimpl.hpp>
#include <gspc/drts/virtual_memory.fwd.hpp>

#include <gspc/we/type/value.hpp>

#include <gspc/iml/SegmentAndAllocation.hpp>

#include <filesystem>
#include <iostream>
#include <string>

namespace gspc
{
  namespace vmem
  {
    using beegfs_segment_description = gspc::iml::beegfs::SegmentDescription;
    using gaspi_segment_description = gspc::iml::gaspi::SegmentDescription;

    using segment_description = gspc::iml::SegmentDescription;
  }

  class GSPC_EXPORT vmem_allocation
  {
  private:
    friend class scoped_runtime_system;

    gspc::iml::SegmentAndAllocation _alloc;

    vmem_allocation (gspc::iml::SegmentAndAllocation alloc);
    gspc::iml::SegmentAndAllocation const& iml_allocation() const
    {
      return _alloc;
    }

  public:
    std::size_t size() const;
    gspc::pnet::type::value::value_type global_memory_range() const;
    gspc::pnet::type::value::value_type global_memory_range ( std::size_t offset
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
