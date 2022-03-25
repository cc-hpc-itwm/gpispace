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

#include <xml/parse/type/expression.hpp>

#include <util-generic/join.hpp>
#include <fhg/util/xml.hpp>

#include <stdexcept>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      expression_type::expression_type ( util::position_type const& pod
                                       , expressions_type const& expressions
                                       )
        : with_position_of_definition (pod)
        , _expressions (expressions)
      {}

      std::string expression_type::expression() const
      {
        return fhg::util::join (_expressions, "; ").string();
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , expression_type const& e
                  )
        {
          s.open ("expression");

          const std::string exp (e.expression ());

          if (exp.size() > 0)
            {
              s.content (exp);
            }

          s.close ();
        }
      }
    }
  }
}
