// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_PROP_HPP
#define _EXPR_TOKEN_PROP_HPP

#include <we/expr/token/type.hpp>

namespace expr
{
  namespace token
  {
    bool is_builtin (const type&);
    bool is_prefix (const type&);
    bool next_can_be_unary (const type&);
    bool is_define (const type&);
    bool is_or (const type&);
    bool is_and (const type&);
  }
}

#endif
