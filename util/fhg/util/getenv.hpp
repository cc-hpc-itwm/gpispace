#ifndef FHG_UTIL_GETENV_HPP
#define FHG_UTIL_GETENV_HPP 1

#include <stdlib.h>
#include <string>

namespace fhg
{
namespace util
{
  std::string getenv(std::string const & s, std::string const & def = "")
  {
    char *val = ::getenv(s.c_str());
    if (val)
    {
      return val;
    }
    else
    {
      return def;
    }
  }
}
}

#endif
