// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_CONNECT_HPP
#define _XML_PARSE_TYPE_CONNECT_HPP

#include <xml/parse/id/generic.hpp>
#include <xml/parse/util/parent.hpp>

#include <xml/parse/type/transition.fwd.hpp>

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
        PARENT_SIGNATURES(transition);

        //! \todo Should be a id::place and id::port. In principle yes
        //! but we do have connections to not yet parsed places
        std::string _place;
        std::string _port;

        std::string _name;

        we::type::property::type _properties;

      public:
        connect_type ( ID_CONS_PARAM(connect)
                     , PARENT_CONS_PARAM(transition)
                     , const std::string& place
                     , const std::string& port
                     );

        const std::string& place() const;
        const std::string& port() const;
        const std::string& name() const;

        const std::string& place (const std::string&);

        const we::type::property::type& properties() const;
        we::type::property::type& properties();
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
