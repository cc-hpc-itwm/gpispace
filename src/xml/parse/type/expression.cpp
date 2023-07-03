// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/type/expression.hpp>

#include <fhg/util/xml.hpp>
#include <util-generic/join.hpp>

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
