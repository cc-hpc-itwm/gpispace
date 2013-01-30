// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_STAT_HPP
#define _FHG_UTIL_STAT_HPP

#include <iosfwd>

#include <string>

namespace fhg
{
  namespace util
  {
    namespace stat
    {
      void out (std::ostream&);
      void inc (const std::string&, const std::string&);
    }
  }
}

#endif
