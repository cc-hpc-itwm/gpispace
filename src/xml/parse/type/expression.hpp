// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
