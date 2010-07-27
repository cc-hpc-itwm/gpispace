// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_USE_HPP
#define _XML_PARSE_TYPE_USE_HPP

#include <xml/parse/types.hpp>
#include <xml/parse/util/maybe.hpp>

#include <iostream>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      // ******************************************************************* //

      struct use_type
      {
        std::string name;
        int level;

        use_type ( const std::string & _name
                 , const int & _level
                 ) 
          : name (_name) 
          , level (_level)
        {}
      };

      std::ostream & operator << (std::ostream & s, const use_type & u)
      {
        return s << level (u.level) << "use (" << std::endl
                 << level (u.level + 1) << "name = " << u.name << std::endl
                 << level (u.level) << ") // use"
          ;
      }
    }
  }
}

#endif
