// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
