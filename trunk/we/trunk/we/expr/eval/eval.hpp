// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_EVAL_HPP
#define _EXPR_EVAL_EVAL_HPP

#include <we/expr/eval/context.hpp>
#include <we/expr/parse/node.hpp>
#include <we/expr/token/function.hpp>

namespace expr
{
  namespace eval
  {
    template<typename T>
    T eval (const parse::node::type<T> & node, context<T> & context)
    {
      if (node.is_value)
        return node.value;

      if (node.is_refname)
        return context.value (node.refname);

      if (node.is_unary)
        return token::function::unary<T> ( node.token
                                         , eval (*node.child0, context)
                                         );

      if (node.is_binary)
        {
          if (is_define (node.token))
            return context.bind ( (*node.child0).refname
                                , eval (*node.child1, context)
                                );
          else
            return token::function::binary<T> ( node.token
                                              , eval (*node.child0, context)
                                              , eval (*node.child1, context)
                                              );
        }

      throw parse::node::unknown();
    }
  }
}

#endif
