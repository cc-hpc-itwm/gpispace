// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/type/eureka.hpp>

#include <xml/parse/type/transition.hpp>
#include <xml/parse/util/position.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      eureka_type::eureka_type ( util::position_type const& pod
                               , std::string const& port
                               )
        : with_position_of_definition (pod)
        , _port (port)
      {}

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, eureka_type const& r)
        {
          s.open ("connect-eureka");
          s.attr ("port", r.port());

          s.close();
        }
      }
    }
  }
}
