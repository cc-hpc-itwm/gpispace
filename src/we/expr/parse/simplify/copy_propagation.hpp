#pragma once

#include <we/expr/parse/simplify/expression_list.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      void copy_propagation (expression_list&);
    }
  }
}
