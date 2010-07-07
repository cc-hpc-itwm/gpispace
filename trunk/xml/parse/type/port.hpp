// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PORT_HPP
#define _XML_PARSE_TYPE_PORT_HPP

#include <string>
#include <iostream>

#include <parse/util/maybe.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct port
      {
      public:
        std::string name;
        std::string type;
        maybe<std::string> place;

        port () : name (), type (), place () {}

        port ( const std::string & _name
             , const std::string & _type
             , const maybe<std::string> & _place
             )
          : name (_name)
          , type (_type)
          , place (_place)
        {}
      };

      std::ostream & operator << (std::ostream & s, const port & p)
      {
        return s << "port ("
                 << "name = " << p.name
                 << ", type = " << p.type
                 << ", place = " << p.place
                 << ")"
          ;
      }
    }
  }
}

#endif
