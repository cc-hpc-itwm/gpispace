// Copyright (C) 2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/AllocationHandle.hpp>
#include <gspc/iml/MemoryOffset.hpp>
#include <gspc/detail/export.hpp>

namespace gspc::iml
{
  //! A location in a local or global allocation with an offset into
  //! that allocation.
  struct GSPC_EXPORT MemoryLocation
  {
    AllocationHandle allocation;
    MemoryOffset offset;

    //! Create a memory location \a offset_ bytes into \a allocation_.
    MemoryLocation (AllocationHandle allocation_, MemoryOffset offset_);

    //! \note For serialization only.
    MemoryLocation() = default;

    //! Serialize using Boost.Serialization.
    template<typename BoostArchive>
      void serialize (BoostArchive&, unsigned int);
  };
}

#include <gspc/iml/MemoryLocation.ipp>
