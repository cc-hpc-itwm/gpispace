// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_PROP_HPP
#define _EXPR_TOKEN_PROP_HPP

#include <we/expr/token/type.hpp>

namespace expr
{
  namespace token
  {
    inline bool is_builtin (const type & token)
    {
      return (token > neg && token < sep);
    }

    inline bool is_prefix (const type & token)
    {
      switch (token)
        {
        case min:
        case max:
        case _floor:
        case _ceil:
        case _round:
        case _sin:
        case _cos:
        case _bitset_insert:
        case _bitset_delete:
        case _bitset_is_element:
        case _context_clear:
        case _substr:
        case abs: return true;
        default: return false;
        }
    }

    inline bool next_can_be_unary (const type & token)
    {
      switch (token)
        {
        case val:
        case rpr:
        case ref: return false;
        default: return true;
        }
    }

    inline bool is_define (const type & token)
    {
      return (token == define);
    }

    inline bool is_context_clear (const type & token)
    {
      return (token == _context_clear);
    }
  }
}

#endif
