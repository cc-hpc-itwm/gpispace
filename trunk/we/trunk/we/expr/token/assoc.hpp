// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_ASSOC_HPP
#define _EXPR_ASSOC_HPP

#include <we/expr/token/type.hpp>

#include <we/util/show.hpp>

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
        case token::mod: return left;
        case token::pow: 
        case token::neg: return right;
        case token::define: return left;
        default: throw std::runtime_error ("associativity " + show(token));
        }
    }
  }
}

#endif
