// Copyright (C) 2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/SharedMemoryAllocationHandle.hpp>

namespace gspc::iml
{
  SharedMemoryAllocationHandle::SharedMemoryAllocationHandle
      (AllocationHandle raw_handle)
    : AllocationHandle (raw_handle)
  {}
}
