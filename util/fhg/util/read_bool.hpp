// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_READ_BOOL_HPP
#define _FHG_UTIL_READ_BOOL_HPP 1

#include <string>

namespace fhg
{
  namespace util
  {
    //! \note Same as fhg::util::parse::require::boolean, but with ::tolower()
    bool read_bool (const std::string&);
  }
}

#endif
