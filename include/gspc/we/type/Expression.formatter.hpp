// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/Expression.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<gspc::we::type::Expression> : fmt::ostream_formatter
  {};
}
