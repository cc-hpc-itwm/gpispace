// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef FHG_UTIL_SYSTEM_WITH_BLOCKED_SIGCHLD_HPP
#define FHG_UTIL_SYSTEM_WITH_BLOCKED_SIGCHLD_HPP

#include <string>

namespace fhg
{
  namespace util
  {
    int system_with_blocked_SIGCHLD (const char*);

    void system_with_blocked_SIGCHLD_or_throw (std::string const&);
  }
}

#endif
