// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/expr/type/Type.hpp>

#include <fmt/ostream.h>

namespace fmt
{
  template<> struct formatter<gspc::we::expr::type::Control> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Boolean> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Int> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Long> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::UInt> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::ULong> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Float> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Double> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Char> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::String> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Bitset> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Bytearray> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Bigint> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Shared> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::List> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Set> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Map> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Struct> : fmt::ostream_formatter{};
  template<> struct formatter<gspc::we::expr::type::Types> : fmt::ostream_formatter{};
}
