// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_PROP_HPP
#define _EXPR_TOKEN_PROP_HPP

#include <we/expr/token/type.hpp>

namespace expr
{
  namespace token
  {
    static bool is_builtin (const type & token)
    {
      return (token > neg && token < sep);
    }

    static bool is_prefix (const type & token)
    {
      switch (token)
        {
        case fac:
        case com:
        case min:
        case max:
        case _floor:
        case _ceil:
        case _sin:
        case _cos:
        case abs: return true;
        default: return false;
        }
    }

    static bool next_can_be_unary (const type & token)
    {
      switch (token)
        {
        case val:
        case rpr:
        case ref: return false;
        default: return true;
        }
    }

    static bool is_define (const type & token)
    {
      return (token == define);
    }
  }
}

#endif
