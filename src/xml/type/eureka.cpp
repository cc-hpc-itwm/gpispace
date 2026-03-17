// Copyright (C) 2020,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/eureka.hpp>

#include <gspc/xml/parse/type/transition.hpp>
#include <gspc/xml/parse/util/position.hpp>

#include <gspc/util/xml.hpp>



    namespace gspc::xml::parse::type
    {
      eureka_type::eureka_type ( util::position_type const& pod
                               , std::string const& port
                               )
        : with_position_of_definition (pod)
        , _port (port)
      {}

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream& s, eureka_type const& r)
        {
          s.open ("connect-eureka");
          s.attr ("port", r.port());

          s.close();
        }
      }
    }
