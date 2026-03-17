// Copyright (C) 2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/MemoryLocation.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/detail/export.hpp>

namespace gspc::iml
{
  //! A region in a local or global allocation with a location and
  //! size.
  struct GSPC_EXPORT MemoryRegion : MemoryLocation
  {
    MemorySize size;

    //! Create a memory region of \a size_ bytes at \a location_.
    MemoryRegion (MemoryLocation location_, MemorySize size_);

    //! \note For serialization only.
    MemoryRegion() = default;

    //! Serialize using Boost.Serialization.
    template<typename BoostArchive>
      void serialize (BoostArchive&, unsigned int);
  };
}

#include <gspc/iml/MemoryRegion.ipp>
