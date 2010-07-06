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
      struct connect
      {
      public:
        std::string place;
        std::string port;
        boost::filesystem::path path;

        connect ( const std::string & _place
                , const std::string & _port
                , const boost::filesystem::path & _path
                )
          : place (_place)
          , port (_port)
          , path (_path)
        {}
      };

      std::ostream & operator << (std::ostream & s, const connect & c)
      {
        return s << "connect ("
                 << "place = " << c.place 
                 << ", port = " << c.port
                 << ", path = " << c.path
                 << ")"
          ;
      }
    }
  }
}

#endif
