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

#include <we/type/Expression.hpp>

#include <we/expr/type/AssignResult.hpp>
#include <we/exception.hpp>

#include <util-generic/nest_exceptions.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <ostream>

namespace we
{
  namespace type
  {
    Expression::Expression ()
      : _expr ("")
      , _ast ("")
    {}

    Expression::Expression (std::string const& expr)
      : _expr (expr)
      , _ast (expr)
   {
     ::boost::trim (_expr);
   }

    // should correspond!
    Expression::Expression (std::string const& expr, expr::parse::parser const& ast)
      : _expr (expr)
      , _ast (ast)
    {
      ::boost::trim (_expr);
    }

    std::string const& Expression::expression() const
    {
      return _expr;
    }

    expr::parse::parser const& Expression::ast() const
    {
      return _ast;
    }

    void Expression::rename (std::string const& from, std::string const& to)
    {
      _ast.rename (from, to);
      _expr = _ast.string();
    }

    expr::Type Expression::type (expr::type::Context& context) const
    {
      return _ast.type_check_all (context);
    }

    void Expression::assert_type
      ( expr::Type const& expected
      , expr::type::Context context
      ) const
    {
      auto const _type (type (context));

      fhg::util::nest_exceptions<pnet::exception::type_error>
        ( [&]
          {
            (void) expr::type::assign_result ({}, expected, _type);
          }
        , str ( ::boost::format
                   ("Expression '%1%' has incompatible type '%2%'."
                   " Expected type '%3%'."
                   )
               % expression()
               % _type
               % expected
               )
        );
    }

    std::ostream& operator<< (std::ostream& os, Expression const& e)
    {
      return os << e.expression();
    }
  }
}
