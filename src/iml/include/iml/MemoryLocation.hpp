// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/AllocationHandle.hpp>
#include <iml/MemoryOffset.hpp>
#include <iml/detail/dllexport.hpp>

namespace iml
{
  //! A location in a local or global allocation with an offset into
  //! that allocation.
  struct IML_DLLEXPORT MemoryLocation
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

#include <iml/MemoryLocation.ipp>
