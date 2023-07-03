// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/type/port.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>

#include <we/type/signature/is_literal.hpp>

#include <fhg/assert.hpp>
#include <util-generic/cxx17/holds_alternative.hpp>

#include <boost/format.hpp>

#include <tuple>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      port_type::unique_key_type::unique_key_type
        ( std::string name
        , we::type::PortDirection port_direction
        )
          : _name (name)
          , _port_direction (port_direction)
      {}
      bool operator== ( port_type::unique_key_type const& lhs
                      , port_type::unique_key_type const& rhs
                      )
      {
#define ESSENCE(x) std::tie (x._name, x._port_direction)
        return ESSENCE (lhs) == ESSENCE (rhs);
#undef ESSENCE
      }
      std::size_t port_type::unique_key_type::hash_value() const
      {
        return std::hash<std::string>{}
          (str (::boost::format ("%1%%2%") % _name % _port_direction));
      }

      port_type::port_type ( util::position_type const& position_of_definition
                           , std::string const& name
                           , std::string const& type
                           , ::boost::optional<std::string> const& _place
                           , we::type::PortDirection const& direction
                           , we::type::property::type const& properties
                           , ::boost::optional<pnet::type::signature::signature_type> signature
                           )
        : with_position_of_definition (position_of_definition)
        , _name (name)
        , _type (type)
        , _signature (std::move (signature))
        , place (_place)
        , _direction (direction)
        , _properties (properties)
      {}

      std::string const& port_type::name() const
      {
        return _name;
      }

      std::string const& port_type::type() const
      {
        return _type;
      }

      pnet::type::signature::signature_type port_type::signature() const
      {
        //! \note assume post processing pass (resolve_types_recursive)
        return _signature.get();
      }
      void port_type::resolve_types_recursive
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
            throw error::port_with_unknown_type
              (direction(), name(), type(), position_of_definition().path());
          }
          _signature = it->second;
        }
      }

      port_type port_type::specialized ( type::type_map_type const& map_in
                                       , state::type const&
                                       ) const
      {
        const type::type_map_type::const_iterator
          mapped (map_in.find (type()));

        return port_type ( position_of_definition()
                         , name()
                         , mapped != map_in.end() ? mapped->second : type()
                         , place
                         , direction()
                         , properties()
                         , _signature
                         );
      }

      namespace
      {
        class type_checker : public ::boost::static_visitor<void>
        {
        private:
          port_type const& _port;
          ::boost::filesystem::path const& _path;
          state::type const& _state;

        public:
          type_checker ( port_type const& port
                       , ::boost::filesystem::path const& path
                       , state::type const& state
                       )
            : _port (port)
            , _path (path)
            , _state (state)
          { }

          void operator() (net_type const& net) const
          {
            if (not _port.place)
            {
              if (_port.is_input())
              {
                _state.warn (warning::port_not_connected (_port, _path));
              }
              else
              {
                throw error::port_not_connected (_port, _path);
              }
            }
            else
            {
              ::boost::optional<place_type const&>
                place (_port.resolved_place (net));

              if (not place)
              {
                throw error::port_connected_place_nonexistent (_port, _path);
              }

              if (place->signature() != _port.signature())
              {
                throw error::port_connected_type_error (_port, *place, _path);
              }

              if (_port.is_tunnel())
              {
                if (not place->is_virtual())
                {
                  throw
                    error::tunnel_connected_non_virtual (_port, *place, _path);
                }

                if (_port.name() != place->name())
                {
                  throw error::tunnel_name_mismatch (_port, *place, _path);
                }
              }
            }
          }

          void operator() (expression_type const&) const
          {
            if (_port.place)
            {
              throw error::port_connected_place_nonexistent (_port, _path);
            }
          }

          void operator() (module_type const&) const
          {
            if (_port.place)
            {
              throw error::port_connected_place_nonexistent (_port, _path);
            }
          }

          void operator() (multi_module_type const&) const
          {
            if (_port.place)
            {
              throw error::port_connected_place_nonexistent (_port, _path);
            }
          }
        };
      }

      void port_type::type_check ( ::boost::filesystem::path const& path
                                 , state::type const& state
                                 , function_type const& parent
                                 ) const
      {
        ::boost::apply_visitor
          (type_checker (*this, path, state), parent.content());
      }

      we::type::PortDirection const& port_type::direction() const
      {
        return _direction;
      }

      ::boost::optional<place_type const&> port_type::resolved_place
        (net_type const& parent) const
      {
        if (!place)
        {
          return ::boost::none;
        }

        return parent.places().get (*place);
      }

      we::type::property::type const& port_type::properties() const
      {
        return _properties;
      }

      port_type::unique_key_type port_type::unique_key() const
      {
        return {name(), direction()};
      }

      bool port_type::is_input() const
      {
        return fhg::util::cxx17::holds_alternative<we::type::port::direction::In> (_direction);
      }
      bool port_type::is_output() const
      {
        return fhg::util::cxx17::holds_alternative<we::type::port::direction::Out> (_direction);
      }
      bool port_type::is_tunnel() const
      {
        return fhg::util::cxx17::holds_alternative<we::type::port::direction::Tunnel> (_direction);
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, port_type const& p)
        {
          s.open (str (::boost::format ("%1%") % p.direction()));
          s.attr ("name", p.name());
          s.attr ("type", p.type());
          s.attr ("place", p.place);

          ::we::type::property::dump::dump (s, p.properties());

          s.close();
        }
      }
    }
  }
}
