// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PREC_HPP
#define _EXPR_PREC_HPP

#include <we/expr/token/type.hpp>
#include <we/expr/exception.hpp>

#include <fhg/util/show.hpp>

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
        case token::divint:
        case token::div: return 22;
        case token::modint:
        case token::mod: return 23;
        case token::_pow:
        case token::_powint: return 24;
        case token::neg: return 25;
        case token::_if:
        case token::_then:
        case token::_else:
        case token::_endif: return -99;
        case token::define: return -98;
        default: throw exception::strange ("prec " + fhg::util::show(token));
        }
    }
  }
}

#endif
