// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_CONNECT_HPP
#define _XML_PARSE_TYPE_CONNECT_HPP

#include <xml/parse/id/mapper.fwd.hpp>
#include <xml/parse/id/types.hpp>

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
                     , id::mapper* id_mapper
                     );

        const id::connect& id() const;
        const id::transition& parent() const;

        bool is_same (const connect_type& other) const;

        //! \todo Should be private with accessors.
      public:
        //! \todo Should be a id::place and id::port.
        std::string place;
        std::string port;

        std::string _name;

        we::type::property::type prop;

        const std::string& name() const;

      private:
        id::connect _id;
        id::transition _parent;
        id::mapper* _id_mapper;
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
