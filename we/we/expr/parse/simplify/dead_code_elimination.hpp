// bernd.loerwald@itwm.fraunhofer.de

#ifndef WE_EXPR_PARSE_SIMPLIFY_DEAD_CODE_ELIMINATION_HPP
#define WE_EXPR_PARSE_SIMPLIFY_DEAD_CODE_ELIMINATION_HPP

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

#endif
