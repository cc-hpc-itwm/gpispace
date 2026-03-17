// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/util/position.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<gspc::xml::parse::util::position_type> : ostream_formatter
  {};
}
