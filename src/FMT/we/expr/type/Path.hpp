// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/type/Path.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<> struct formatter<expr::type::Path> : fmt::ostream_formatter{};
}
