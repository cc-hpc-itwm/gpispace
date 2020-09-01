#pragma once

#include <string>

namespace fhg
{
  namespace util
  {
    void system_with_blocked_SIGCHLD (std::string const&);
  }
}
