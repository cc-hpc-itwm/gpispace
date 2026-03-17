// Copyright (C) 2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/MemoryRegion.hpp>

namespace gspc::iml
{
  MemoryRegion::MemoryRegion
      (MemoryLocation location_, MemorySize size_)
    : MemoryLocation (location_)
    , size (size_)
  {}
}
