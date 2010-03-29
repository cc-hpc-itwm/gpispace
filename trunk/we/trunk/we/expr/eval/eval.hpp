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
    template<typename Key, typename Value>
    Value eval ( const parse::node::type<Key,Value> & node
               , context<Key,Value> & context
               )
    {
      if (node.is_value)
        return node.value;

      if (node.is_ref)
        return context.value (node.ref);

      if (node.is_unary)
        return token::function::unary<Value> ( node.token
                                             , eval (*node.child0, context)
                                             );

      if (node.is_binary)
        {
          if (is_define (node.token))
            return context.bind ( (*node.child0).ref
                                , eval (*node.child1, context)
                                );
          else
            return token::function::binary<Value> ( node.token
                                                  , eval (*node.child0, context)
                                                  , eval (*node.child1, context)
                                                  );
        }

      throw parse::node::unknown();
    }
  }
}

#endif
