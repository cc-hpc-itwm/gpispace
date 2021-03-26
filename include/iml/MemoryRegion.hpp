// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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
