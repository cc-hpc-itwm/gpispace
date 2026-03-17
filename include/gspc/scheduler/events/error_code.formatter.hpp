// Copyright (C) 2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/ErrorEvent.hpp>

#include <fmt/format.h>

namespace fmt
{
  template<>
    struct formatter
      <gspc::scheduler::events::ErrorEvent::error_code_t>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( gspc::scheduler::events::ErrorEvent::error_code_t
            const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/error_code.formatter.ipp"
