#ifndef FHG_UTIL_GETENV_HPP
#define FHG_UTIL_GETENV_HPP 1

#include <boost/optional.hpp>

#include <cstdlib>
#include <string>

namespace fhg
{
  namespace util
  {
    boost::optional<const char*> getenv (const char* env_var)
    {
      const char *val (std::getenv (env_var));

      if (val)
      {
        return val;
      }

      return boost::none;
    }
  }
}

#endif
