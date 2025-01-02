// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/variant.hpp>
#include <fmt/core.h>
#include <util-generic/functor_visitor.hpp>

namespace fmt
{
  template<typename... Ts>
    struct formatter<::boost::variant<Ts...>>
  {
    template<typename ParseContext>
      [[nodiscard]] constexpr auto parse (ParseContext& ctx) const
    {
      return ctx.begin();
    }

    template<typename FormatContext>
      [[nodiscard]] constexpr auto format
        ( ::boost::variant<Ts...> const& v
        , FormatContext& ctx
        ) const -> decltype (ctx.out())
    {
      return fhg::util::visit<decltype (ctx.out())>
        ( v
        , [&] (auto const& x)
          {
            return fmt::format_to (ctx.out(), "{}", x);
          }
        );
    }
  };
}
