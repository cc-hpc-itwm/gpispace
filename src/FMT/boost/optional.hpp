// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/optional.hpp>
#include <fmt/core.h>

namespace fmt
{
  template<typename T>
    struct formatter<::boost::optional<T>>
  {
    template<typename ParseContext>
      [[nodiscard]] constexpr auto parse (ParseContext& ctx) const
    {
      return ctx.begin();
    }

    template<typename FormatContext>
      [[nodiscard]] constexpr auto format
        ( ::boost::optional<T> const& o
        , FormatContext& ctx
        ) const -> decltype (ctx.out())
    {
      return !!o
        ? fmt::format_to (ctx.out(), "{}", *o)
        : fmt::format_to (ctx.out(), "Nothing")
        ;
    }
  };
}
