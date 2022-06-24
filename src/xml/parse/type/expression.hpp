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

#pragma once

#include <xml/parse/type/expression.fwd.hpp>

#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <list>
#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      //! \todo Move this into class scope.
      using expressions_type = std::list<std::string>;

      struct expression_type : with_position_of_definition
      {
      public:
        expression_type (util::position_type const&, expressions_type const&);

        std::string expression() const;

      private:
        expressions_type _expressions;
      };

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , expression_type const& e
                  );
      }
    }
  }
}
