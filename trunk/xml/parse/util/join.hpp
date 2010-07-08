// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_JOIN_HPP
#define _XML_PARSE_UTIL_JOIN_HPP

#include <string>
#include <sstream>

namespace xml
{
  namespace parse
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
    }
  }
}

#endif
