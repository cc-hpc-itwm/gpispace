// mirko.rahn@itwm.fraunhofer.de

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
        case token::define: return -98;
        default: throw std::runtime_error
            ((boost::format ("prec (%1%)") % expr::token::show (token)).str());
        }
    }
  }
}
