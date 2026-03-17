// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/AllocationHandle.hpp>
#include <gspc/iml/SharedMemoryAllocationHandle.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<gspc::iml::AllocationHandle> : ostream_formatter
  {};
  template<>
    struct formatter<gspc::iml::SharedMemoryAllocationHandle> : ostream_formatter
  {};
}
