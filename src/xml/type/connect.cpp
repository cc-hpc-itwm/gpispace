// Copyright (C) 2012-2014,2016,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/connect.hpp>
#include <gspc/xml/parse/type/net.hpp>
#include <gspc/xml/parse/type/transition.hpp>

#include <gspc/util/xml.hpp>

#include <gspc/we/type/net.formatter.hpp>
#include <fmt/core.h>



    namespace gspc::xml::parse::type
    {
      connect_type::connect_type ( util::position_type const& pod
                                 , std::string const& place
                                 , std::string const& port
                                 , const ::gspc::we::edge::type& direction
                                 , ::gspc::we::type::property::type const& properties
                                 )
        : with_position_of_definition (pod)
        , _place (place)
        , _port (port)
        , _direction (direction)
        , _properties (properties)
      {}

      std::string const& connect_type::place() const
      {
        return _place;
      }
      std::string const& connect_type::port() const
      {
        return _port;
      }

      const ::gspc::we::edge::type& connect_type::direction() const
      {
        return _direction;
      }

      connect_type connect_type::with_place (std::string const& place) const
      {
        return connect_type ( position_of_definition()
                            , place
                            , _port
                            , _direction
                            , _properties
                            );
      }

      ::gspc::we::type::property::type const& connect_type::properties() const
      {
        return _properties;
      }

      connect_type::unique_key_type connect_type::unique_key() const
      {
        return std::make_tuple
          (_place, _port, ::gspc::we::edge::is_incoming (_direction));
      }

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream& s, connect_type const& c)
        {
          s.open (fmt::format ("connect-{}", c.direction()));
          s.attr ("port", c.port());
          s.attr ("place", c.place());

          ::gspc::we::type::property::dump::dump (s, c.properties());

          s.close ();
        }
      }
    }
