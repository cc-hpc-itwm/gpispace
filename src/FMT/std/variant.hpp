// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <fmt/core.h>
#include <variant>

namespace fmt
{
  template<typename... Ts>
    struct formatter<std::variant<Ts...>>
  {
    static_assert ((is_formattable<Ts>::value && ...));

    template<typename ParseContext>
      [[nodiscard]] constexpr auto parse (ParseContext& context) const
    {
      return context.begin();
    }

    template<typename FormatContext>
      [[nodiscard]] constexpr auto format
        ( std::variant<Ts...> const& variant
        , FormatContext& context
        ) const
    {
      return std::visit
        ( [&] (auto const& value)
          {
            return fmt::format_to (context.out(), "{}", value);
          }
        , variant
        );
    }
  };
}

#include <iostream>
#include <fmt/ostream.h>

namespace std
{
  template<typename... Ts>
    auto operator<< (ostream& os, variant<Ts...> const& variant) -> ostream&
  {
    fmt::print (os, "{}", variant);

    return os;
  }
}
