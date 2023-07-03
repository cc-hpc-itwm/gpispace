// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/MemoryLocation.hpp>
#include <iml/MemorySize.hpp>
#include <iml/detail/dllexport.hpp>

namespace iml
{
  //! A region in a local or global allocation with a location and
  //! size.
  struct IML_DLLEXPORT MemoryRegion : MemoryLocation
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

#include <iml/MemoryRegion.ipp>
