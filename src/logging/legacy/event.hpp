#pragma once

#include <fhglog/event.hpp>

namespace fhg
{
  namespace logging
  {
    namespace legacy
    {
      using namespace ::fhg::log;

      constexpr const char* const category_level_trace = "legacy-level-trace";
      constexpr const char* const category_level_info = "legacy-level-info";
      constexpr const char* const category_level_warn = "legacy-level-warn";
      constexpr const char* const category_level_error = "legacy-level-error";

      using event = LogEvent;
    }
  }
}
