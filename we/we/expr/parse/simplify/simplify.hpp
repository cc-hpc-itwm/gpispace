// bernd.loerwald@itwm.fraunhofer.de

#ifndef WE_EXPR_PARSE_SIMPLIFY_SIMPLIFY_HPP
#define WE_EXPR_PARSE_SIMPLIFY_SIMPLIFY_HPP

#include <we/expr/parse/parser.hpp>
#include <we/expr/parse/util/get_names.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      typedef ::expr::parse::util::name_set_t key_set_type;

      parser simplification_pass
        (const parser&, const key_set_type& needed_bindings);
    }
  }
}

#endif
