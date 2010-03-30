// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PREC_HPP
#define _EXPR_PREC_HPP

#include <we/expr/token/type.hpp>

#include <we/util/show.hpp>

namespace expr
{
  namespace prec
  {
    typedef int type;

    static type prec (const token::type & token)
    {
      switch (token)
        {
        case token::_or: return 0;
        case token::_and: return 1;
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
        case token::div: return 22;
        case token::mod: return 23;
        case token::pow: return 24;
        case token::neg: return 25;
        case token::_if:
        case token::_then:
        case token::_else:
        case token::_endif: return -98;
        case token::define: return -99;
        default: throw std::runtime_error ("prec " + show(token));
        }
    }
  }
}

#endif
