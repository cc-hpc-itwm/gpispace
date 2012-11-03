// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_CONNECT_HPP
#define _XML_PARSE_TYPE_CONNECT_HPP

#include <xml/parse/id/generic.hpp>

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

        boost::optional<id::transition> _parent;

        //! \todo Should be a id::place and id::port.
        std::string _place;
        std::string _port;

        std::string _name;

      public:
        connect_type ( ID_CONS_PARAM(connect)
                     , const id::transition& parent
                     , const std::string& place
                     , const std::string& port
                     );

        bool has_parent() const;
        boost::optional<const transition_type&> parent() const;
        boost::optional<transition_type&> parent();

        const std::string& place() const;
        const std::string& port() const;
        const std::string& name() const;

        const std::string& place (const std::string&);

        //! \todo Should be private with accessors.
        we::type::property::type prop;
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
