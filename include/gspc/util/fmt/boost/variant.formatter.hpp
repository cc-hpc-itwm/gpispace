// Copyright (C) 2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/variant.hpp>
#include <fmt/format.h>

namespace fmt
{
  template<typename... Ts>
    struct formatter<::boost::variant<Ts...>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( ::boost::variant<Ts...> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/variant.formatter.ipp"
