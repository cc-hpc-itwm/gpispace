// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_READ_BOOL_HPP
#define _XML_PARSE_UTIL_READ_BOOL_HPP

#include <parse/rapidxml/1.13/rapidxml.hpp>

namespace xml
{
  namespace parse
  {
    bool
    read_bool (const std::string & inp)
    {
      if (inp == "true")
        {
          return true;
        }
      else if (inp == "false")
        {
          return false;
        }
      else
        {
          throw std::runtime_error ("failed to read a bool from: " + inp);
        }
    }
  }
}

#endif
