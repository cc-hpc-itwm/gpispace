// Copyright (C) 2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace fmt
{
  template<typename T>
    template<typename ParseContext>
      constexpr auto
        formatter<std::optional<T>>::parse
          (ParseContext& ctx)
  {
    return ctx.begin();
  }

  template<typename T>
    template<typename FormatContext>
      constexpr auto
        formatter<std::optional<T>>::format
          ( std::optional<T> const& o
          , FormatContext& ctx
          ) const
            -> decltype (ctx.out())
  {
    return !!o
      ? fmt::format_to (ctx.out(), "{}", *o)
      : fmt::format_to (ctx.out(), "Nothing")
      ;
  }
}
