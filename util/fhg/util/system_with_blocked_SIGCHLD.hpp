// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef FHG_UTIL_SYSTEM_WITH_BLOCKED_SIGCHLD_HPP
#define FHG_UTIL_SYSTEM_WITH_BLOCKED_SIGCHLD_HPP

#include <string>

namespace fhg
{
  namespace util
  {
    void system_with_blocked_SIGCHLD (std::string const&);
  }
}

#endif
