// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/AllocationHandle.hpp>
#include <iml/SharedMemoryAllocationHandle.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<iml::AllocationHandle> : ostream_formatter
  {};
  template<>
    struct formatter<iml::SharedMemoryAllocationHandle> : ostream_formatter
  {};
}
