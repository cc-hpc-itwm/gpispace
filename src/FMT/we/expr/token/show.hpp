// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/token/type.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<expr::token::show> : fmt::ostream_formatter
  {};
}

namespace fmt
{
  template<>
    struct formatter<expr::token::type> : fmt::ostream_formatter
  {};
}
