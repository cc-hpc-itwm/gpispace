// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <string>
#include <sstream>

namespace fhg
{
  namespace util
  {
    template<typename IT>
    std::string join ( IT begin, IT end
                     , const std::string & sep
                     , const std::string & local_open = ""
                     , const std::string & local_close = ""
                     )
    {
      std::ostringstream s;

      if (begin != end)
        {
          s << local_open << *begin << local_close;

          ++begin;
        }

      while (begin != end)
        {
          s << sep << local_open << *begin << local_close;

          ++begin;
        }

      return s.str();
    };

    template<typename V>
    std::string join ( const V & v
                     , const std::string & sep
                     , const std::string & local_open = ""
                     , const std::string & local_close = ""
                     )
    {
      return join (v.begin(), v.end(), sep, local_open, local_close);
    }
  }
}
