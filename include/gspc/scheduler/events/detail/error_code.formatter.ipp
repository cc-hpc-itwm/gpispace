// Copyright (C) 2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/scheduler/events/error_code.formatter.hpp>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto
      formatter
        <gspc::scheduler::events::ErrorEvent::error_code_t>
          ::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }

  template<typename FormatContext>
    constexpr auto
      formatter
        <gspc::scheduler::events::ErrorEvent::error_code_t>
          ::format
            ( gspc::scheduler::events::ErrorEvent
                ::error_code_t const& ec
            , FormatContext& ctx
            ) const
              -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}"
      , ec == gspc::scheduler::events::ErrorEvent::SCHEDULER_ENODE_SHUTDOWN ? "SCHEDULER_ENODE_SHUTDOWN"
      : ec == gspc::scheduler::events::ErrorEvent::SCHEDULER_EUNKNOWN ? "SCHEDULER_EUNKNOWN"
      : "SCHEDULER_UNKNOWN_ERROR_CODE"
      );
  }
}
