// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <xml/parse/error.hpp>
#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/type_map_type.hpp>

#include <xml/parse/util/position.fwd.hpp>

#include <util-generic/hash/std/pair.hpp>
#include <fhg/util/xml.fwd.hpp>

#include <we/type/port.hpp>
#include <we/type/property.hpp>
#include <we/type/signature.hpp>

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
      public:
        typedef std::pair<std::string, we::type::PortDirection> unique_key_type;

        port_type ( const util::position_type&
                  , const std::string & name
                  , const std::string & _type
                  , const boost::optional<std::string> & _place
                  , const we::type::PortDirection& direction
                  , const we::type::property::type& properties
                  = we::type::property::type()
                  , boost::optional<pnet::type::signature::signature_type> = boost::none
                  );

        port_type specialized ( const type::type_map_type & map_in
                              , const state::type &
                              ) const;

        void type_check ( const boost::filesystem::path&
                        , const state::type&
                        , function_type const& parent
                        ) const;

        const std::string& name() const;

        const std::string& type() const;

        pnet::type::signature::signature_type signature() const;
        void resolve_types_recursive
          (std::unordered_map<std::string, pnet::type::signature::signature_type> known);

        const we::type::PortDirection& direction() const;

        boost::optional<place_type const&> resolved_place
          (id::ref::net const& parent) const;

        const we::type::property::type& properties() const;

        unique_key_type unique_key() const;

      private:
        std::string _name;
        std::string _type;
        boost::optional<pnet::type::signature::signature_type> _signature;

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
