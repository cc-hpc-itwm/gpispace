#pragma once

#include <fmt/format.h>
#include <iostream>
#include <optional>

namespace fmt
{
  template<typename T>
    struct formatter<std::optional<T>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( std::optional<T> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace std
{
  template<typename T>
    auto operator<< (ostream&, optional<T> const&) -> ostream&;
}

#include "detail/optional.formatter.ipp"
