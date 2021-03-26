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

#include <we/expr/token/prec.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace expr
{
  namespace prec
  {
    int prec (const token::type & token)
    {
      switch (token)
        {
        case token::_or_boolean: return 0;
        case token::_or_integral: return 0;
        case token::_and_boolean: return 1;
        case token::_and_integral: return 1;
        case token::_not: return 30;
        case token::lt:
        case token::le:
        case token::gt:
        case token::ge:
        case token::ne:
        case token::eq: return 10;
        case token::add:
        case token::sub: return 21;
        case token::mul:
        case token::divint:
        case token::div: return 22;
        case token::modint: return 23;
        case token::_pow:
        case token::_powint: return 24;
        case token::neg: return 25;
        case token::define: return -98;
        default: throw std::runtime_error
            ((boost::format ("prec (%1%)") % expr::token::show (token)).str());
        }
    }
  }
}
