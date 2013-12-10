// bernd.loerwald@itwm.fraunhofer.de

#ifndef WE_EXPR_PARSE_SIMPLIFY_CONSTANT_PROPAGATION_HPP
#define WE_EXPR_PARSE_SIMPLIFY_CONSTANT_PROPAGATION_HPP

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

#endif
