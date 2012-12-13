// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PORT_HPP
#define _XML_PARSE_TYPE_PORT_HPP

#include <xml/parse/error.hpp>
#include <xml/parse/id/generic.hpp>
#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type_map_type.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/port.hpp>
#include <we/type/property.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct port_type
      {
        ID_SIGNATURES(port);
        PARENT_SIGNATURES(function);

      public:
        typedef std::pair<std::string, we::type::PortDirection> unique_key_type;

        port_type ( ID_CONS_PARAM(port)
                  , PARENT_CONS_PARAM(function)
                  , const std::string & name
                  , const std::string & _type
                  , const boost::optional<std::string> & _place
                  , const we::type::PortDirection& direction
                  , const we::type::property::type& properties
                  = we::type::property::type()
                  );

        void specialize ( const type::type_map_type & map_in
                        , const state::type &
                        );

        void type_check ( const std::string & direction
                        , const boost::filesystem::path & path
                        , const state::type & state
                        ) const;

        const std::string& name() const;

        const we::type::PortDirection& direction() const;
        const we::type::PortDirection& direction (const we::type::PortDirection&);

        const we::type::property::type& properties() const;
        we::type::property::type& properties();

        unique_key_type unique_key() const;

        id::ref::port clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        std::string _name;

        //! \todo All these should be private with accessors.
      public:
        std::string type;
        boost::optional<std::string> place;

      private:
        we::type::PortDirection _direction;
        we::type::property::type _properties;
      };

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const port_type & p
                  , const std::string & direction
                  );
      }
    }
  }
}

#endif
