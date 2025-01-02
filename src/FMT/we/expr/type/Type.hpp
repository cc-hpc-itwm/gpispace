// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/type/Type.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<> struct formatter<expr::type::Control> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::Boolean> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::Int> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::Long> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::UInt> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::ULong> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::Float> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::Double> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::Char> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::String> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::Bitset> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::Bytearray> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::List> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::Set> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::Map> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::Struct> : fmt::ostream_formatter{};
  template<> struct formatter<expr::type::Types> : fmt::ostream_formatter{};
}
