// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <xml/parse/type/place.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/type/net.hpp>

#include <we/type/signature/is_literal.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      place_type::place_type ( util::position_type const& pod
                             , std::string const& name
                             , std::string const& type
                             , ::boost::optional<bool> is_virtual
                             , ::boost::optional<bool> put_token
                             , std::list<token_type> tokens_
                             , we::type::property::type properties
                             , ::boost::optional<pnet::type::signature::signature_type> signature
                             )
        : with_position_of_definition (pod)
        , _is_virtual (is_virtual)
        , _put_token (put_token)
        , _name (name)
        , _type (type)
        , _signature (std::move (signature))
        , tokens (std::move (tokens_))
        , _properties (std::move (properties))
      {}

      std::string const& place_type::name() const
      {
        return _name;
      }

      std::string const& place_type::type() const
      {
        return _type;
      }

      place_type place_type::with_name (std::string const& new_name) const
      {
        return place_type ( position_of_definition()
                          , new_name
                          , _type
                          , _is_virtual
                          , _put_token
                          , tokens
                          , _properties
                          , _signature
                          );
      }

      pnet::type::signature::signature_type place_type::signature() const
      {
        //! \note assume post processing pass (resolve_types_recursive)
        return _signature.get();
      }
      void place_type::resolve_types_recursive
        (std::unordered_map<std::string, pnet::type::signature::signature_type> known)
      {
        if (pnet::type::signature::is_literal (_type))
        {
          _signature = pnet::type::signature::signature_type (_type);
        }
        else
        {
          auto it (known.find (_type));
          if (it == known.end())
          {
            throw error::place_type_unknown (*this);
          }
          _signature = it->second;
        }
      }

      void place_type::push_token (token_type const& t)
      {
        tokens.push_back (t);
      }

      place_type place_type::specialized ( type::type_map_type const& map_in
                                         , state::type const&
                                         ) const
      {
        const type::type_map_type::const_iterator
          mapped (map_in.find (type()));

        return place_type ( position_of_definition()
                          , _name
                          , mapped != map_in.end() ? mapped->second : type()
                          , _is_virtual
                          , _put_token
                          , tokens
                          , _properties
                          , _signature
                          );
      }

      ::boost::optional<bool> const& place_type::get_is_virtual() const
      {
        return _is_virtual;
      }
      bool place_type::is_virtual() const
      {
        return _is_virtual.get_value_or (false);
      }

      we::type::property::type const& place_type::properties() const
      {
        return _properties;
      }
      we::type::property::type& place_type::properties()
      {
        return _properties;
      }

      place_type::unique_key_type const& place_type::unique_key() const
      {
        return name();
      }

      namespace dump
      {
        void dump (fhg::util::xml::xmlstream & s, place_type const& p)
        {
          s.open ("place");
          s.attr ("name", p.name());
          s.attr ("type", p.type());
          s.attr ("virtual", p.get_is_virtual());
          s.attr ("put_token", p.put_token());

          ::we::type::property::dump::dump (s, p.properties());

          for (place_type::token_type const& token : p.tokens)
          {
            s.open ("token");
            s.open ("value");
            s.content (token);
            s.close();
            s.close();
          }

          s.close();
        }
      }
    }
  }
}
