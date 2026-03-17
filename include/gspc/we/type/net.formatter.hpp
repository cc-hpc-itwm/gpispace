// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/net.hpp>

#include <gspc/util/fmt/std/variant.formatter.hpp>
#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<gspc::we::edge::PT> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<gspc::we::edge::PT_READ> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<gspc::we::edge::PT_NUMBER_OF_TOKENS> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<gspc::we::edge::TP> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<gspc::we::edge::TP_MANY> : fmt::ostream_formatter
  {};
}
