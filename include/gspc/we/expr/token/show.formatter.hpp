// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/expr/token/type.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<gspc::we::expr::token::show> : fmt::ostream_formatter
  {};
}

namespace fmt
{
  template<>
    struct formatter<gspc::we::expr::token::type> : fmt::ostream_formatter
  {};
}
