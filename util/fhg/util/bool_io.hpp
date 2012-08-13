#ifndef FHG_UTIL_BOOL_IO_HPP
#define FHG_UTIL_BOOL_IO_HPP

#include <string>
#include <ostream>
#include <istream>

#include <fhg/util/bool.hpp>
#include <fhg/util/read_bool.hpp>

namespace fhg
{
  namespace util
  {
    inline std::ostream & operator<< (std::ostream &os, const bool_t &b)
    {
      os << static_cast<bool>(b);
      return os;
    }

    inline std::istream & operator>> (std::istream &is, bool_t &b)
    {
      std::string s;
      is >> s;
      b = fhg::util::read_bool (s);
      return is;
    }
  }
}

#endif
