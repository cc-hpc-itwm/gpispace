// mirko.rahn@itwm.fraunhofer.de

#ifndef _UTIL_SHOW_HPP
#define _UTIL_SHOW_HPP

#include <string>
#include <sstream>

namespace util
{
  template<typename T>
  inline std::string show (const T & x)
  {
    std::ostringstream s; s << x; return s.str();
  }

  template<>
  inline std::string show<std::string> (const std::string & x)
  {
    return x;
  }
}

#endif
