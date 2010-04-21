// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_EVAL_HPP
#define _EXPR_EVAL_EVAL_HPP

#include <we/expr/eval/context.hpp>
#include <we/expr/parse/node.hpp>

#include <we/type/value.hpp>
#include <we/type/value/function.hpp>

namespace expr
{
  namespace eval
  {
    template<typename Key>
    value::type eval ( const parse::node::type<Key> & node
                     , context<Key> & context
                     )
    {
      switch (node.flag)
        {
        case expr::parse::node::flag::value: return node.value;
        case expr::parse::node::flag::ref: return context.value (node.ref);
        case expr::parse::node::flag::unary:
          if (is_context_clear (node.token))
            {
              return context.clear ((*node.child0).ref);
            }
          else
            {
              value::type c0 (eval (*node.child0, context));

              return boost::apply_visitor ( value::function::unary (node.token)
                                          , c0
                                          );
            }
        case expr::parse::node::flag::binary:
          if (is_define (node.token))
            {
              value::type c1 (eval (*node.child1, context));

              return context.bind ((*node.child0).ref, c1);
            }
          else
            {
              value::type c0 (eval (*node.child0, context));
              value::type c1 (eval (*node.child1, context));

              return boost::apply_visitor ( value::function::binary (node.token)
                                          , c0
                                          , c1
                                          );
            }
        case expr::parse::node::flag::ternary:
          if (node.token == expr::token::_ite)
            {
              if (value::function::is_true (eval (*node.child0, context)))
                return eval (*node.child1, context);
              else
                return eval (*node.child2, context);
            }
          else
            throw expr::exception::strange ("ternary but not ite");

        default: throw parse::node::unknown();
        }
    }
  }
}

#endif
