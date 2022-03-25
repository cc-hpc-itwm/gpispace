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
