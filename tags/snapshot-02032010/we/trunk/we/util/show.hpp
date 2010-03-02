// mirko.rahn@itwm.fraunhofer.de

#ifndef _UTIL_SHOW_HPP
#define _UTIL_SHOW_HPP

#include <string>
#include <sstream>

template<typename T>
std::string show (const T & x)
{
  std::ostringstream s; s << x; return s.str();
}

#endif
