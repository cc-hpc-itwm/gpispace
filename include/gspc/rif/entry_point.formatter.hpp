// Copyright (C) 2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/rif/entry_point.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<gspc::rif::entry_point>
      : ostream_formatter
  {};
}
