// Copyright (C) 2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/MemoryLocation.hpp>

namespace gspc::iml
{
  MemoryLocation::MemoryLocation
      (AllocationHandle allocation_, MemoryOffset offset_)
    : allocation (allocation_)
    , offset (offset_)
  {}
}
