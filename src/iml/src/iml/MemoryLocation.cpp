// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/MemoryLocation.hpp>

namespace iml
{
  MemoryLocation::MemoryLocation
      (AllocationHandle allocation_, MemoryOffset offset_)
    : allocation (allocation_)
    , offset (offset_)
  {}
}
