// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/MemoryRegion.hpp>

namespace iml
{
  MemoryRegion::MemoryRegion
      (MemoryLocation location_, MemorySize size_)
    : MemoryLocation (location_)
    , size (size_)
  {}
}
