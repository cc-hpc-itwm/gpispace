// Copyright (C) 2012-2016,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/expression.hpp>

#include <gspc/util/xml.hpp>
#include <gspc/util/join.hpp>

#include <stdexcept>



    namespace gspc::xml::parse::type
    {
      expression_type::expression_type ( util::position_type const& pod
                                       , expressions_type const& expressions
                                       )
        : with_position_of_definition (pod)
        , _expressions (expressions)
      {}

      std::string expression_type::expression() const
      {
        return gspc::util::join (_expressions, "; ").string();
      }

      namespace dump
      {
        void dump ( ::gspc::util::xml::xmlstream & s
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
