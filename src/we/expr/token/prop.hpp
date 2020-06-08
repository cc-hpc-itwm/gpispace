#pragma once

#include <we/expr/token/type.hpp>

namespace expr
{
  namespace token
  {
    bool is_builtin (const type&);
    bool is_prefix (const type&);
    bool next_can_be_unary (const type&);
    bool is_define (const type&);
    bool is_or_boolean (const type&);
    bool is_and_boolean (const type&);
  }
}
