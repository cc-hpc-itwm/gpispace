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

#include <we/type/Expression.fwd.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/expr/type/Context.hpp>
#include <we/expr/type/Type.hpp>

#include <boost/serialization/utility.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>
#include <iosfwd>

namespace we
{
  namespace type
  {
    struct Expression
    {
      Expression ();
      Expression (std::string const& expr);

      // should correspond!
      Expression (std::string const& expr, expr::parse::parser const& ast);

      std::string const& expression() const;
      expr::parse::parser const& ast() const;

      void rename (std::string const& from, std::string const& to);

      expr::Type type (expr::type::Context&) const;

      void assert_type (expr::Type const&, expr::type::Context) const;

    private:
      std::string _expr;
      expr::parse::parser _ast;

      friend class ::boost::serialization::access;
      template <typename Archive>
      void save (Archive& ar, unsigned int) const
      {
	ar << ::boost::serialization::make_nvp ("expr", _expr);
      }
      template <typename Archive>
      void load (Archive& ar, unsigned int)
      {
	std::string tmp;
	ar >> ::boost::serialization::make_nvp ("expr", tmp);
	_ast = expr::parse::parser (tmp);
	_expr = tmp;
      }
      BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    std::ostream& operator<< (std::ostream&, Expression const&);
  }
}
