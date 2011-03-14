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
        case _stack_empty:
        case _stack_top:
        case _stack_push:
        case _stack_pop:
        case _stack_size:
        case _stack_join:
        case _map_assign:
        case _map_unassign:
        case _map_is_assigned:
        case _map_get_assignment:
        case _set_insert:
        case _set_erase:
        case _set_is_element:
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

    inline bool is_or (const type & token)
    {
      return (token == _or);
    }

    inline bool is_and (const type & token)
    {
      return (token == _and);
    }
  }
}

#endif
