// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/property.hpp>
#include <we/type/signature.hpp>

#include <string>
#include <list>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct place_type : with_position_of_definition
      {
      public:
        typedef std::string unique_key_type;

        typedef std::string token_type;

        place_type ( const util::position_type&
                   , const std::string & name
                   , const std::string & type
                   , const boost::optional<bool> is_virtual
                   , boost::optional<bool> put_token
                   , std::list<token_type> tokens = {}
                   , we::type::property::type properties = {}
                   );

        const std::string& name() const;
        const std::string& type() const;

        place_type with_name (std::string const& new_name) const;

        boost::optional<pnet::type::signature::signature_type> signature
          (net_type const& parent) const;
        pnet::type::signature::signature_type signature_or_throw
          (net_type const& parent) const;

        void push_token (const token_type & t);

        place_type specialized ( const type::type_map_type & map_in
                               , const state::type &
                               ) const;

        const boost::optional<bool>& get_is_virtual (void) const;
        bool is_virtual (void) const;
        boost::optional<bool> const& put_token() const
        {
          return _put_token;
        }

        const we::type::property::type& properties() const;
        we::type::property::type& properties();

        const unique_key_type& unique_key() const;

      private:
        boost::optional<bool> _is_virtual;
        boost::optional<bool> _put_token;

        std::string _name;
        std::string _type;

        //! \todo All these should be private with accessors.
      public:
        std::list<token_type> tokens;

      private:
        we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream & s, const place_type & p);
      }
    }
  }
}
