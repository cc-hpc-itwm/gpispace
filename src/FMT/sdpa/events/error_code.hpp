// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/events/ErrorEvent.hpp>

#include <fmt/core.h>

namespace fmt
{
  template<>
    struct formatter<sdpa::events::ErrorEvent::error_code_t>
  {
    template<typename ParseContext>
      [[nodiscard]] constexpr auto parse (ParseContext& ctx) const
    {
      return ctx.begin();
    }

    template<typename FormatContext>
      [[nodiscard]] constexpr auto format
        ( sdpa::events::ErrorEvent::error_code_t const& ec
        , FormatContext& ctx
        ) const
    {
      return fmt::format_to
        ( ctx.out()
        , "{}"
        , ec == sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN ? "SDPA_ENODE_SHUTDOWN"
        : ec == sdpa::events::ErrorEvent::SDPA_EUNKNOWN ? "SDPA_EUNKNOWN"
        : "SDPA_UNKNOWN_ERROR_CODE"
        );
    }
  };
}
