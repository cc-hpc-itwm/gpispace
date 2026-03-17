// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/join.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>

namespace fmt
{
  template<typename C, typename Separator>
    struct formatter<gspc::util::join_reference<C, Separator>>
      : ostream_formatter
  {};
}
