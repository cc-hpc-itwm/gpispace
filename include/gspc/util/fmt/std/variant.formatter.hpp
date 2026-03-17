#pragma once

#include <fmt/format.h>
#include <iostream>
#include <variant>

namespace fmt
{
  template<typename... Ts>
    struct formatter<std::variant<Ts...>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( std::variant<Ts...> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace std
{
  template<typename... Ts>
    auto operator<< (ostream&, variant<Ts...> const&) -> ostream&;
}

#include "detail/variant.formatter.ipp"
