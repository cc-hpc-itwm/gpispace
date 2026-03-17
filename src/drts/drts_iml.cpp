// Copyright (C) 2020,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/drts/drts_iml.hpp>

#include <gspc/we/type/value.hpp>
#include <gspc/we/type/value/poke.hpp>

namespace gspc::pnet::type::value
{
  template<>
    value_type
      to_value<gspc::iml::AllocationHandle>
        (gspc::iml::AllocationHandle const& handle)
  {
    return handle.to_string();
  }

  template<>
    value_type
      to_value<gspc::iml::MemoryRegion>
        (gspc::iml::MemoryRegion const& region)
  {
    value_type result;

    poke ( "handle.name"
         , result
         , to_value (region.allocation)
         );
    poke ("offset", result, region.offset);
    poke ("size", result, region.size);

    return result;
  }

  template<>
    value_type
      to_value<gspc::StreamSlot>
        (gspc::StreamSlot const& slot)
  {
    value_type result;

    poke ("meta", result, to_value (slot.metadata));
    poke ("data", result, to_value (slot.data));
    poke ("flag", result, slot.flag);
    poke ("id", result, slot.id);

    return result;
  }
}
