// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_CONNECT_HPP
#define _XML_PARSE_TYPE_CONNECT_HPP

#include <xml/parse/util/id_type.hpp>

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
      public:
        connect_type ( const std::string & _place
                     , const std::string & _port
                     , const id::connect& id
                     , const id::transition& parent
                     );

        const id::connect& id() const;
        const id::transition& parent() const;

        bool is_same (const connect_type& other) const;

        //! \todo Should be private with accessors.
      public:
        //! \todo Should be a id::place and id::port.
        std::string place;
        std::string port;

        //! \todo This is only needed for errors and unique<connect_type>.
        std::string name;

        we::type::property::type prop;

      private:
        id::connect _id;
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
