// Copyright (C) 2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>

#include <fmt/format.h>

namespace fmt
{
  template<>
    struct formatter<::std::filesystem::path>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( ::std::filesystem::path const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/path.formatter.ipp"
