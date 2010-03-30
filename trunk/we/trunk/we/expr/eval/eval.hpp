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
      switch (node.flag)
        {
        case expr::parse::node::flag::value: return node.value;
        case expr::parse::node::flag::ref: return context.value (node.ref);
        case expr::parse::node::flag::unary:
          return token::function::unary<Value> ( node.token
                                               , eval (*node.child0, context)
                                               );
        case expr::parse::node::flag::binary:
          if (is_define (node.token))
            return context.bind ( (*node.child0).ref
                                , eval (*node.child1, context)
                                );
          else
            return token::function::binary<Value> ( node.token
                                                  , eval (*node.child0, context)
                                                  , eval (*node.child1, context)
                                                  );
        default: throw parse::node::unknown();
        }
    }
  }
}

#endif
