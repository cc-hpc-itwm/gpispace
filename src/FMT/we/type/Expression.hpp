// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/Expression.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<we::type::Expression> : fmt::ostream_formatter
  {};
}
