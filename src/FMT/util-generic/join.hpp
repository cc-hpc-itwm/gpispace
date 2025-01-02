// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/join.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>

namespace fmt
{
  template<typename C, typename Separator>
    struct formatter<fhg::util::join_reference<C, Separator>>
      : ostream_formatter
  {};
}
