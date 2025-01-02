// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/net.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<we::edge::PT> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<we::edge::PT_READ> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<we::edge::PT_NUMBER_OF_TOKENS> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<we::edge::TP> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<we::edge::TP_MANY> : fmt::ostream_formatter
  {};
}
