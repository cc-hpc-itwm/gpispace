// Copyright (C) 2015-2016,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/response.hpp>

#include <gspc/xml/parse/type/transition.hpp>
#include <gspc/xml/parse/util/position.hpp>

#include <gspc/util/xml.hpp>



    namespace gspc::xml::parse::type
    {
      response_type::response_type ( util::position_type const& pod
                                   , std::string const& port
                                   , std::string const& to
                                   , ::gspc::we::type::property::type const& properties
                                   )
        : with_position_of_definition (pod)
        , _port (port)
        , _to (to)
        , _properties (properties)
      {}

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream& s, response_type const& r)
        {
          s.open ("connect-response");
          s.attr ("port", r.port());
          s.attr ("to", r.to());

          ::gspc::we::type::property::dump::dump (s, r.properties());

          s.close();
        }
      }
    }
