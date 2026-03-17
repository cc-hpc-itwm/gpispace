// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/value/show.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<gspc::pnet::type::value::show> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<gspc::pnet::type::bitsetofint::type> : fmt::ostream_formatter
  {};
}
