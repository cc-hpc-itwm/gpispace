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

#include <drts/drts_iml.hpp>

#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>

#include <sstream>

namespace gspc
{
  namespace pnet
  {
    namespace vmem
    {
      ::pnet::type::value::value_type handle_to_value
        (iml::AllocationHandle handle)
      {
        return handle.to_string();
      }

      ::pnet::type::value::value_type memory_region_to_value
        (iml::MemoryRegion region)
      {
        ::pnet::type::value::value_type result;

        ::pnet::type::value::poke
            ("handle.name", result, handle_to_value (region.allocation));
        ::pnet::type::value::poke ("offset", result, region.offset);
        ::pnet::type::value::poke ("size", result, region.size);

        return result;
      }

      ::pnet::type::value::value_type stream_slot_to_value
        ( iml::MemoryRegion const& metadata
        , iml::MemoryRegion const& data
        , char const flag
        , std::size_t const id
        )
      {
        ::pnet::type::value::value_type result;

        ::pnet::type::value::poke
            ("meta", result, memory_region_to_value (metadata));
        ::pnet::type::value::poke
            ("data", result, memory_region_to_value (data));
        ::pnet::type::value::poke ("flag", result, flag);
        ::pnet::type::value::poke ("id", result, id);

        return result;
      }
    }
  }
}
