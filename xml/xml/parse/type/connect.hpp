// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_CONNECT_HPP
#define _XML_PARSE_TYPE_CONNECT_HPP

#include <xml/parse/type/id.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/property.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct connect_type
      {
        ID_SIGNATURES(connect);

      public:
        connect_type ( ID_CONS_PARAM(connect)
                     , const std::string & _place
                     , const std::string & _port
                     , const id::transition& parent
                     );

        const id::transition& parent() const;

        //! \todo Should be private with accessors.
      public:
        //! \todo Should be a id::place and id::port.
        std::string place;
        std::string port;

        std::string _name;

        we::type::property::type prop;

        const std::string& name() const;

      private:
        id::transition _parent;
      };

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const connect_type & c
                  , const std::string & type
                  );
      }
    }
  }
}

#endif
