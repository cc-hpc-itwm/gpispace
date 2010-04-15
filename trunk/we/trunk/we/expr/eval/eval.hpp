// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_EVAL_HPP
#define _EXPR_EVAL_EVAL_HPP

#include <we/expr/eval/context.hpp>
#include <we/expr/parse/node.hpp>
#include <we/type/literal.hpp>
#include <we/type/literal/function.hpp>

namespace expr
{
  namespace eval
  {
    template<typename Key>
    literal::type eval ( const parse::node::type<Key> & node
                       , context<Key> & context
                       )
    {
      switch (node.flag)
        {
        case expr::parse::node::flag::value: return node.value;
        case expr::parse::node::flag::ref: return context.value (node.ref);
        case expr::parse::node::flag::unary:
          {
            literal::type c (eval (*node.child0, context));

            return boost::apply_visitor ( literal::function::unary (node.token)
                                        , c
                                        );
          }
        case expr::parse::node::flag::binary:
          if (is_define (node.token))
            return context.bind ( (*node.child0).ref
                                , eval (*node.child1, context)
                                );
          else
            {
              literal::type l (eval (*node.child0, context));
              literal::type r (eval (*node.child1, context));

              return boost::apply_visitor ( literal::function::binary (node.token)
                                          , l
                                          , r
                                          );
            }
        case expr::parse::node::flag::ternary:
          return literal::function::ternary ( node.token
                                          , eval (*node.child0, context)
                                          , eval (*node.child1, context)
                                          , eval (*node.child2, context)
                                          );

        default: throw parse::node::unknown();
        }
    }
  }
}

#endif
