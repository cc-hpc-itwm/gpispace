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

  template <typename InputIterator>
  inline std::string show (InputIterator first, InputIterator last, const std::string & delimiter = ", ", const std::string & start_end = "[]")
  {
    InputIterator start (first);
    std::ostringstream s;

    if (start_end.size() == 2)
      s << start_end[0];

    while (first != last)
    {
      if (first != start)
        s << delimiter;
      s << *first;
      ++first;
    }

    if (start_end.size() == 2)
      s << start_end[1];

    return s.str();
  }
}

#endif
