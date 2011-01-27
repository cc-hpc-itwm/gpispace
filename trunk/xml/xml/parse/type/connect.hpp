// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_CONNECT_HPP
#define _XML_PARSE_TYPE_CONNECT_HPP

#include <string>
#include <iostream>

#include <boost/filesystem.hpp>

#include <we/type/property.hpp>

#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

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
        we::type::property::type prop;
        int level;

        connect_type ( const std::string & _place
                     , const std::string & _port
                     , const int & _level
                     )
          : place (_place)
          , port (_port)
          , name (_place + " <-> " + _port)
          , level (_level)
        {}
      };

      namespace dump
      {
        inline void dump ( xml_util::xmlstream & s
                         , const connect_type & c
                         , const std::string & type
                         )
        {
          s.open ("connect-" + type);
          s.attr ("port", c.port);
          s.attr ("place", c.place);

          ::we::type::property::dump::dump (s, c.prop);

          s.close ();
        }
      } // namespace dump

      std::ostream & operator << (std::ostream & s, const connect_type & c)
      {
        s << level(c.level)  << "connect (" << std::endl;
        s << level(c.level+1) << "place = " << c.place << std::endl;
        s << level(c.level+1) << "port = " << c.port << std::endl;
        s << level(c.level+1) << "properties = " << std::endl;

        c.prop.writeTo (s, c.level+2);

        return s << level(c.level) << ") // connect";
      }

      typedef std::vector<connect_type> connect_vec_type;
    }
  }
}

#endif
