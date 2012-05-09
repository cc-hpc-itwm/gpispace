// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_READ_BOOL_HPP
#define _FHG_UTIL_READ_BOOL_HPP 1

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    inline bool
    read_bool (const std::string& _inp)
    {
      std::string inp;

      std::transform ( _inp.begin(), _inp.end()
                     , std::back_inserter (inp), tolower
                     );

      if (inp == "true" || inp == "yes" || inp == "1" || inp == "on")
        {
          return true;
        }

      if (inp == "false" || inp == "no" || inp == "0" || inp == "off")
        {
          return false;
        }

      throw std::runtime_error ("failed to read a bool from: '" + _inp + "'");
    }
  }
}

#endif
