// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_USE_HPP
#define _XML_PARSE_TYPE_USE_HPP

#include <xml/parse/types.hpp>

#include <iostream>

#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

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

      namespace dump
      {
        void inline dump (xml_util::xmlstream & s, const use_type & u)
        {
          s.open ("use");
          s.attr ("name", u.name);
          s.close ();
        }
      }

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
