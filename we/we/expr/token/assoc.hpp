// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_ASSOC_HPP
#define _EXPR_ASSOC_HPP

#include <we/expr/token/type.hpp>

namespace expr
{
  namespace associativity
  {
    enum type {left, right};

    type associativity (const token::type&);
  }
}

#endif
