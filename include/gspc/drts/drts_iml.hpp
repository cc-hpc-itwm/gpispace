// Copyright (C) 2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/type/value.hpp>
#include <gspc/we/type/value/to_value.hpp>

#include <gspc/iml/AllocationHandle.hpp>
#include <gspc/iml/MemoryRegion.hpp>

#include <gspc/drts/StreamSlot.hpp>

namespace gspc::pnet::type::value
{
  template<>
    GSPC_EXPORT value_type
      to_value<gspc::iml::AllocationHandle>
        (gspc::iml::AllocationHandle const&);

  template<>
    GSPC_EXPORT value_type
      to_value<gspc::iml::MemoryRegion>
        (gspc::iml::MemoryRegion const&);

  template<>
    GSPC_EXPORT value_type
      to_value<gspc::StreamSlot>
        (gspc::StreamSlot const&);
}
