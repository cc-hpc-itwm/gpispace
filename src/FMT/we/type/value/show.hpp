// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/value/show.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<pnet::type::value::show> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<bitsetofint::type> : fmt::ostream_formatter
  {};
}
