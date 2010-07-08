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
      std::string join (IT begin, IT end, const std::string & sep)
      {
        std::ostringstream s;

        if (begin != end)
          {
            s << *begin;

            ++begin;
          }

        while (begin != end)
          {
            s << sep << *begin;

            ++begin;
          }

        return s.str();
      };
    }
  }
}

#endif
