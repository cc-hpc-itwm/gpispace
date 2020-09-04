// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <we/type/expression.hpp>

#include <boost/algorithm/string.hpp>

#include <ostream>

namespace we
{
  namespace type
  {
    expression_t::expression_t ()
      : _expr ("")
      , _ast ("")
    {}

    expression_t::expression_t (const std::string& expr)
      : _expr (expr)
      , _ast (expr)
   {
     boost::trim (_expr);
   }

    // should correspond!
    expression_t::expression_t (const std::string& expr, const expr::parse::parser& ast)
      : _expr (expr)
      , _ast (ast)
    {
      boost::trim (_expr);
    }

    const std::string& expression_t::expression() const
    {
      return _expr;
    }

    const expr::parse::parser& expression_t::ast() const
    {
      return _ast;
    }

    bool expression_t::simplify
    (const expr::parse::simplify::key_set_type& needed_bindings)
    {
      _ast = expr::parse::parser (expr::parse::simplify::simplification_pass
        (_ast, needed_bindings));

      //! \todo Eliminate this comparsion, get the modified flag from
      //! the call to the simplification pass
      const bool modified (_ast.string() != _expr);

      if (modified)
      {
        _expr = _ast.string();
      }

      return modified;
    }

    void expression_t::rename (const std::string& from, const std::string& to)
    {
      _ast.rename (from, to);
      _expr = _ast.string();
    }

    std::ostream& operator<< (std::ostream& os, const expression_t& e)
    {
      return os << e.expression();
    }
  }
}
