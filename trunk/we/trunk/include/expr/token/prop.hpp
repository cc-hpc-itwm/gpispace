// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_PROP_HPP
#define _EXPR_TOKEN_PROP_HPP

#include <expr/token/type.hpp>

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
  }
}

#endif
