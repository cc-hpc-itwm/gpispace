// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/Port.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<we::type::port::direction::In> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<we::type::port::direction::Out> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<we::type::port::direction::Tunnel> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<we::type::PortDirection> : fmt::ostream_formatter
  {};
}
