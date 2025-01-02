// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

// \todo Move to gspc/pnet/vmem.hpp.

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <we/type/value.hpp>

#include <iml/AllocationHandle.hpp>
#include <iml/MemoryRegion.hpp>

namespace gspc
{
  namespace pnet
  {
    namespace vmem
    {
      //! Convert an IML memory allocation \a handle to a petri net
      //! token usable for memory transfers.
      GSPC_DLLEXPORT ::pnet::type::value::value_type handle_to_value
        (iml::AllocationHandle handle);

      //! Convert an IML memory \a region to a petri net token usable
      //! for memory transfers.
      GSPC_DLLEXPORT ::pnet::type::value::value_type memory_region_to_value
        (iml::MemoryRegion region);

      //! Convert a stream slot information to a petri net token.
      GSPC_DLLEXPORT ::pnet::type::value::value_type stream_slot_to_value
        ( iml::MemoryRegion const& metadata
        , iml::MemoryRegion const& data
        , char flag
        , std::size_t id
        );
    }
  }
}
