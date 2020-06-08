#pragma once

#include <we/expr/token/type.hpp>

namespace expr
{
  namespace associativity
  {
    enum type {left, right};

    type associativity (const token::type&);
  }
}
