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
