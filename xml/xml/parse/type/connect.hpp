// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_CONNECT_HPP
#define _XML_PARSE_TYPE_CONNECT_HPP

#include <string>
#include <iostream>

#include <xml/parse/util/id_type.hpp>

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

      private:
        id::connect _id;
        id::transition _parent;

      public:
        connect_type ( const std::string & _place
                     , const std::string & _port
                     , const id::connect& id
                     , const id::transition& parent
                     )
          : place (_place)
          , port (_port)
          , name (_place + " <-> " + _port)
          , _id (id)
          , _parent (parent)
        {}

        const id::connect& id() const
        {
          return _id;
        }
        const id::transition& parent() const
        {
          return _parent;
        }

        bool is_same (const connect_type& other) const
        {
          return id() == other.id() && parent() == other.parent();
        }
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
    }
  }
}

#endif
