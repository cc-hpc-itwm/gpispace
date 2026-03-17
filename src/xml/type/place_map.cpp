// Copyright (C) 2012-2013,2016,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/place_map.hpp>

#include <gspc/xml/parse/type/net.hpp>
#include <gspc/xml/parse/type/transition.hpp>

#include <gspc/util/xml.hpp>



    namespace gspc::xml::parse::type
    {
      place_map_type::place_map_type ( util::position_type const& pod
                                     , std::string const& place_virtual
                                     , std::string const& place_real
                                     , ::gspc::we::type::property::type const& prop
                                     )
        : with_position_of_definition (pod)
        , _place_virtual (place_virtual)
        , _place_real (place_real)
        , _properties (prop)
      {}

      std::string const& place_map_type::place_virtual() const
      {
        return _place_virtual;
      }
      std::string const& place_map_type::place_real() const
      {
        return _place_real;
      }

      place_map_type place_map_type::with_place_real (std::string const& place_real) const
      {
        return place_map_type ( position_of_definition()
                              , _place_virtual
                              , place_real
                              , _properties
                              );
      }

      ::gspc::we::type::property::type const& place_map_type::properties() const
      {
        return _properties;
      }

      place_map_type::unique_key_type place_map_type::unique_key() const
      {
        return std::make_pair (place_virtual(), place_real());
      }

      namespace dump
      {
        void dump ( ::gspc::util::xml::xmlstream & s
                  , place_map_type const& p
                  )
        {
          s.open ("place-map");
          s.attr ("virtual", p.place_virtual());
          s.attr ("real", p.place_real());

          ::gspc::we::type::property::dump::dump (s, p.properties());

          s.close ();
        }
      }
    }
