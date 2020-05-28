#pragma once

#include <we/expr/parse/simplify/expression_list.hpp>
#include <we/expr/parse/simplify/simplify.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      void dead_code_elimination
        (expression_list&, const key_set_type& needed_bindings);
    }
  }
}
