// mirko.rahn@itwm.fraunhofer.de

#ifndef _UTIL_READ_HPP
#define _UTIL_READ_HPP

#include <string>
#include <sstream>

namespace we { namespace util {
  template<typename T>
  inline T read (const std::string & showed)
  {
    T x;

    std::istringstream i(showed); 

    i >> x;

    return x;
  }

  template<>
  inline std::string read (const std::string & s)
  {
    return s;
  }

  template<typename I>
  inline I read_int (const std::string & s)
  {
    std::string::const_iterator pos (s.begin());
    const std::string::const_iterator end (s.end());

    I sign (1);

    if (*pos == '-')
      {
        sign = -1;
        ++pos;
      }

    I x (0);

    while (isdigit(*pos))
      {
        x *= 10;
        x += *pos - '0';
        ++pos;
      }

    return (sign > 0) ? x : -x;
  }
}}

#endif
