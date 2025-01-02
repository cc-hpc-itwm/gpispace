// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/SharedMemoryAllocationHandle.hpp>

namespace iml
{
  SharedMemoryAllocationHandle::SharedMemoryAllocationHandle
      (AllocationHandle raw_handle)
    : AllocationHandle (raw_handle)
  {}
}
