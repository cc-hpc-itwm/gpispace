// Copyright (C) 2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/fmt/boost/variant.formatter.hpp>
#include <gspc/util/functor_visitor.hpp>

namespace fmt
{
  template<typename... Ts>
    template<typename ParseContext>
      constexpr auto
        formatter<::boost::variant<Ts...>>::parse
          (ParseContext& ctx)
  {
    return ctx.begin();
  }

  template<typename... Ts>
    template<typename FormatContext>
      constexpr auto
        formatter<::boost::variant<Ts...>>::format
          ( ::boost::variant<Ts...> const& v
          , FormatContext& ctx
          ) const
            -> decltype (ctx.out())
  {
    return gspc::util::visit<decltype (ctx.out())>
      ( v
      , [&] (auto const& x)
        {
          return fmt::format_to (ctx.out(), "{}", x);
        }
      );
  }
}
