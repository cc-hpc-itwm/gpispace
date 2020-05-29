#pragma once

#include <we/expr/parse/simplify/expression_list.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      void constant_propagation (expression_list&);
    }
  }
}
