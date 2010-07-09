// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_CONNECT_HPP
#define _XML_PARSE_TYPE_CONNECT_HPP

#include <string>
#include <iostream>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct connect_type
      {
      public:
        std::string place;
        std::string port;
        std::string name;

        connect_type ( const std::string & _place
                     , const std::string & _port
                     )
          : place (_place)
          , port (_port)
          , name (_place + " <-> " + _port)
        {}
      };

      std::ostream & operator << (std::ostream & s, const connect_type & c)
      {
        return s << "connect ("
                 << "place = " << c.place 
                 << ", port = " << c.port
                 << ")"
          ;
      }

      typedef std::vector<connect_type> connect_vec_type;
    }
  }
}

#endif
