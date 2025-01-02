// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/eval/context.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<expr::eval::context> : fmt::ostream_formatter
  {};
}
