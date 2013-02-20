// bernd.loerwald@itwm.fraunhofer.de

#include <we/expr/parse/simplify/simplify.hpp>

#include <we/expr/parse/simplify/constant_propagation.hpp>
#include <we/expr/parse/simplify/copy_propagation.hpp>
#include <we/expr/parse/simplify/dead_code_elimination.hpp>
#include <we/expr/parse/simplify/expression_list.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      parser simplification_pass
        (const parser& parser, const key_set_type& needed_bindings)
      {
        expression_list expressions (parser);

        constant_propagation (expressions);
        copy_propagation (expressions);
        dead_code_elimination (expressions, needed_bindings);

        return parse::parser (expressions.nodes());
      }
    }
  }
}
