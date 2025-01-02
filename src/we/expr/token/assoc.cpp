// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
        case token::neg:
        case token::define: return right;

        default: return left;
        }
    }
  }
}
