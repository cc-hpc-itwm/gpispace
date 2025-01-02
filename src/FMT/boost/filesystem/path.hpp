// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem.hpp>

#include <fmt/core.h>

namespace fmt
{
  template<>
    struct formatter<::boost::filesystem::path>
  {
    template<typename ParseContext>
      [[nodiscard]] constexpr auto parse (ParseContext& ctx) const
    {
      return ctx.begin();
    }

    template<typename FormatContext>
      [[nodiscard]] constexpr auto format
        ( ::boost::filesystem::path const& path
        , FormatContext& ctx
        ) const
    {
      return fmt::format_to (ctx.out(), "{}", path.string());
    }
  };
}
