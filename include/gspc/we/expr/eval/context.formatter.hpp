// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/expr/eval/context.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<gspc::we::expr::eval::context> : fmt::ostream_formatter
  {};
}
