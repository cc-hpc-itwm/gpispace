// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/Port.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <variant>

namespace fmt
{
  template<>
    struct formatter<gspc::we::type::port::direction::In> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<gspc::we::type::port::direction::Out> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<gspc::we::type::port::direction::Tunnel> : fmt::ostream_formatter
  {};
  template<>
    struct formatter<gspc::we::type::PortDirection>
  {
    constexpr auto parse (format_parse_context& context)
    {
      return context.begin();
    }
    template<typename FormatContext>
      auto format
        ( gspc::we::type::PortDirection const& v
        , FormatContext& context
        ) const
    {
      return std::visit
        ( [&] (auto const& d)
          {
            return fmt::format_to (context.out(), "{}", d);
          }
        , v
        );
    }
  };
}
