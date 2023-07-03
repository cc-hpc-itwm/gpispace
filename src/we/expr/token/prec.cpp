// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/expr/token/prec.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace expr
{
  namespace prec
  {
    int prec (token::type const& token)
    {
      switch (token)
        {
        case token::_or_boolean:
        case token::_or_integral: return 0;
        case token::_and_boolean:
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
            ((::boost::format ("prec (%1%)") % expr::token::show (token)).str());
        }
    }
  }
}
