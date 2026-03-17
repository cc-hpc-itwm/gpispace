// Copyright (C) 2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto
      formatter<::std::filesystem::path>::parse
        (ParseContext& ctx)
  {
    return ctx.begin();
  }

  template<typename FormatContext>
    constexpr auto
      formatter<::std::filesystem::path>::format
        ( ::std::filesystem::path const& path
        , FormatContext& ctx
        ) const -> decltype (ctx.out())
  {
    return fmt::format_to (ctx.out(), "{}", path.string());
  }
}
