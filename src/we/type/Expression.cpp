// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/Expression.hpp>

#include <we/exception.hpp>
#include <we/expr/type/AssignResult.hpp>

#include <boost/algorithm/string.hpp>

#include <FMT/boost/variant.hpp>
#include <FMT/we/expr/type/Type.hpp>
#include <exception>
#include <fmt/core.h>
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

      try
      {
        (void) expr::type::assign_result ({}, expected, _type);
      }
      catch (...)
      {
        std::throw_with_nested
          ( pnet::exception::type_error
            { fmt::format
              ( "Expression '{}' has incompatible type '{}'."
                " Expected type '{}'."
              , expression()
              , _type
              , expected
              )
            }
          );
      }
    }

    std::ostream& operator<< (std::ostream& os, Expression const& e)
    {
      return os << e.expression();
    }
  }
}
