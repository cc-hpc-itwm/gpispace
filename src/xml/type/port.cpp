// Copyright (C) 2012-2016,2019-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/port.hpp>

#include <gspc/xml/parse/error.hpp>
#include <gspc/xml/parse/state.hpp>
#include <gspc/xml/parse/type/net.hpp>
#include <gspc/xml/parse/type/place.hpp>

#include <gspc/we/type/signature/is_literal.hpp>

#include <gspc/assert.hpp>
#include <variant>

#include <gspc/we/type/port/direction.formatter.hpp>
#include <fmt/core.h>
#include <tuple>
#include <optional>



    namespace gspc::xml::parse::type
    {
      port_type::unique_key_type::unique_key_type
        ( std::string name
        , ::gspc::we::type::PortDirection port_direction
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
          (fmt::format ("{}{}", _name, _port_direction));
      }

      port_type::port_type ( util::position_type const& position_of_definition
                           , std::string const& name
                           , std::string const& type
                           , std::optional<std::string> const& _place
                           , ::gspc::we::type::PortDirection const& direction
                           , ::gspc::we::type::property::type const& properties
                           , std::optional<gspc::pnet::type::signature::signature_type> signature
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

      gspc::pnet::type::signature::signature_type port_type::signature() const
      {
        //! \note assume post processing pass (resolve_types_recursive)
        return _signature.value();
      }
      void port_type::resolve_types_recursive
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
          std::filesystem::path const& _path;
          state::type const& _state;

        public:
          type_checker ( port_type const& port
                       , std::filesystem::path const& path
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
              ::std::optional<std::reference_wrapper<place_type const>>
                place (_port.resolved_place (net));

              if (not place)
              {
                throw error::port_connected_place_nonexistent (_port, _path);
              }

              if (place->get().signature() != _port.signature())
              {
                throw error::port_connected_type_error (_port, *place, _path);
              }

              if (_port.is_tunnel())
              {
                if (not place->get().is_virtual())
                {
                  throw
                    error::tunnel_connected_non_virtual (_port, *place, _path);
                }

                if (_port.name() != place->get().name())
                {
                  throw error::tunnel_name_mismatch (_port, place->get(), _path);
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

      void port_type::type_check ( std::filesystem::path const& path
                                 , state::type const& state
                                 , function_type const& parent
                                 ) const
      {
        ::boost::apply_visitor
          (type_checker (*this, path, state), parent.content());
      }

      ::gspc::we::type::PortDirection const& port_type::direction() const
      {
        return _direction;
      }

      std::optional<std::reference_wrapper<place_type const>> port_type::resolved_place
        (net_type const& parent) const
      {
        if (!place)
        {
          return {};
        }

        return parent.places().get (*place);
      }

      ::gspc::we::type::property::type const& port_type::properties() const
      {
        return _properties;
      }

      port_type::unique_key_type port_type::unique_key() const
      {
        return {name(), direction()};
      }

      bool port_type::is_input() const
      {
        return std::holds_alternative<::gspc::we::type::port::direction::In>
          ( _direction
          );
      }
      bool port_type::is_output() const
      {
        return std::holds_alternative<::gspc::we::type::port::direction::Out>
          ( _direction
          );
      }
      bool port_type::is_tunnel() const
      {
        return std::holds_alternative<::gspc::we::type::port::direction::Tunnel>
          ( _direction
          );
      }

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream& s, port_type const& p)
        {
          s.open (fmt::format ("{}", p.direction()));
          s.attr ("name", p.name());
          s.attr ("type", p.type());
          s.attr ("place", p.place);

          ::gspc::we::type::property::dump::dump (s, p.properties());

          s.close();
        }
      }
    }
