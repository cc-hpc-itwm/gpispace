// Copyright (C) 2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <exception>
#include <fmt/format.h>

namespace fmt
{
  template<>
    struct formatter<std::exception_ptr>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( std::exception_ptr const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };

  template<>
    struct formatter<std::exception>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( std::exception const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/exception.formatter.ipp"
