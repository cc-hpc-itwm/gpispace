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

#include <we/expr/token/assoc.hpp>

namespace expr
{
  namespace associativity
  {
    type associativity (token::type const& token)
    {
      switch (token)
        {
        case token::_or_boolean:
        case token::_or_integral:
        case token::_and_boolean:
        case token::_and_integral: return left;
        case token::_not: return right;
        case token::lt:
        case token::le:
        case token::gt:
        case token::ge:
        case token::ne:
        case token::eq:
        case token::add:
        case token::sub:
        case token::mul:
        case token::div:
        case token::divint:
        case token::modint: return left;
        case token::_pow:
        case token::_powint:
        case token::neg: return right;
        case token::define: return right;

        default: return left;
        }
    }
  }
}
