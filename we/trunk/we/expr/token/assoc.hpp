// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_ASSOC_HPP
#define _EXPR_ASSOC_HPP

#include <we/expr/token/type.hpp>

#include <we/expr/exception.hpp>

#include <fhg/util/show.hpp>

namespace expr
{
  namespace associativity
  {
    enum type {left, right};

    static type associativity (const token::type & token)
    {
      switch (token)
        {
        case token::_or:
        case token::_and: return left;
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
        case token::modint:
        case token::mod: return left;
        case token::_pow:
        case token::_powint:
        case token::neg: return right;
        case token::define: return right;
        case token::_endif: return left;

        default: return left;
        }
    }
  }
}

#endif
