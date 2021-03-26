// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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
