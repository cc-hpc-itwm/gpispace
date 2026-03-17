// Copyright (C) 2012-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/place.hpp>

#include <gspc/xml/parse/error.hpp>
#include <gspc/xml/parse/type/net.hpp>

#include <gspc/we/type/signature/is_literal.hpp>

#include <gspc/util/xml.hpp>
#include <optional>



    namespace gspc::xml::parse::type
    {
      place_type::place_type ( util::position_type const& pod
                             , std::string const& name
                             , std::string const& type
                             , std::optional<bool> is_virtual
                             , std::optional<bool> put_token
                             , std::optional<bool> shared_sink
                             , std::optional<bool> generator
                             , std::list<token_type> tokens_
                             , ::gspc::we::type::property::type properties
                             , std::optional<gspc::pnet::type::signature::signature_type> signature
                             )
        : with_position_of_definition (pod)
        , _is_virtual (is_virtual)
        , _put_token (put_token)
        , _shared_sink (shared_sink)
        , _generator (generator)
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
                          , _shared_sink
                          , _generator
                          , tokens
                          , _properties
                          , _signature
                          );
      }

      gspc::pnet::type::signature::signature_type place_type::signature() const
      {
        //! \note assume post processing pass (resolve_types_recursive)
        return _signature.value();
      }
      void place_type::resolve_types_recursive
        (std::unordered_map<std::string, gspc::pnet::type::signature::signature_type> known)
      {
        if (gspc::pnet::type::signature::is_literal (_type))
        {
          _signature = gspc::pnet::type::signature::signature_type (_type);
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
                          , _shared_sink
                          , _generator
                          , tokens
                          , _properties
                          , _signature
                          );
      }

      std::optional<bool> const& place_type::get_is_virtual() const
      {
        return _is_virtual;
      }
      bool place_type::is_virtual() const
      {
        return _is_virtual.value_or (false);
      }

      ::gspc::we::type::property::type const& place_type::properties() const
      {
        return _properties;
      }
      ::gspc::we::type::property::type& place_type::properties()
      {
        return _properties;
      }

      place_type::unique_key_type const& place_type::unique_key() const
      {
        return name();
      }

      namespace dump
      {
        void dump (gspc::util::xml::xmlstream & s, place_type const& p)
        {
          s.open ("place");
          s.attr ("name", p.name());
          s.attr ("type", p.type());
          s.attr ("virtual", p.get_is_virtual());
          s.attr ("put_token", p.put_token());
          s.attr ("shared_sink", p.shared_sink());
          s.attr ("generator", p.generator());

          ::gspc::we::type::property::dump::dump (s, p.properties());

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
