// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_CONNECT_HPP
#define _XML_PARSE_TYPE_CONNECT_HPP

#include <string>
#include <iostream>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct connect
      {
      public:
        const std::string place;
        const std::string port;

        connect ( const std::string & _place
                , const std::string & _port
                )
          : place (_place)
          , port (_port)
        {}
      };

      std::ostream & operator << (std::ostream & s, const connect & c)
      {
        return s << "connect ("
                 << "place = " << c.place 
                 << ", port = " << c.port
                 << ")"
          ;
      }
    }
  }
}

#endif
