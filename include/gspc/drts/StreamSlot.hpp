// Copyright (C) 2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/MemoryRegion.hpp>

#include <cstddef>

namespace gspc
{
  struct StreamSlot
  {
    iml::MemoryRegion metadata;
    iml::MemoryRegion data;
    char flag;
    std::size_t id;
  };
}
