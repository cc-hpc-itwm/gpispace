// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_EVAL_HPP
#define _EXPR_EVAL_EVAL_HPP

#include <we/expr/eval/context.hpp>
#include <we/expr/parse/node.hpp>

namespace expr
{
  namespace eval
  {
    value::type eval (context&, const expr::parse::node::type&);
  }
}

#endif
