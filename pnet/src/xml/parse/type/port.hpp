// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PORT_HPP
#define _XML_PARSE_TYPE_PORT_HPP

#include <xml/parse/error.hpp>
#include <xml/parse/id/generic.hpp>
#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/type_map_type.hpp>

#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/port.hpp>
#include <we/type/property.hpp>
#include <we2/type/signature.hpp>

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
      struct port_type : with_position_of_definition
      {
        ID_SIGNATURES(port);
        PARENT_SIGNATURES(function);

      public:
        typedef std::pair<std::string, we::type::PortDirection> unique_key_type;

        port_type ( ID_CONS_PARAM(port)
                  , PARENT_CONS_PARAM(function)
                  , const util::position_type&
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

        void type_check
          (const boost::filesystem::path&, const state::type&) const;

        const std::string& name() const;
        const std::string& name (const std::string& name);

        const std::string& type() const;
        const std::string& type (const std::string&);

        boost::optional<signature::type> signature() const;
        signature::type signature_or_throw() const;

        boost::optional<pnet::type::signature::signature_type> signature2() const;
        pnet::type::signature::signature_type signature2_or_throw() const;

        const we::type::PortDirection& direction() const;
        const we::type::PortDirection& direction (const we::type::PortDirection&);

      private:
        friend struct function_type;
        const std::string& name_impl (const std::string& name);
        const we::type::PortDirection& direction_impl
          (const we::type::PortDirection&);

      public:
        boost::optional<const id::ref::place&> resolved_place() const;

        const we::type::property::type& properties() const;
        we::type::property::type& properties();

        unique_key_type unique_key() const;

        id::ref::port clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        std::string _name;
        std::string _type;

        //! \todo All these should be private with accessors.
      public:
        boost::optional<std::string> place;

      private:
        we::type::PortDirection _direction;
        we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, const port_type&);
      }
    }
  }
}

#endif
