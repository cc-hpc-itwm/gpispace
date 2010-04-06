// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_EVAL_HPP
#define _EXPR_EVAL_EVAL_HPP

#include <we/expr/eval/context.hpp>
#include <we/expr/parse/node.hpp>
#include <we/expr/token/function.hpp>
#include <we/expr/variant/variant.hpp>

namespace expr
{
  namespace eval
  {
    template<typename Key>
    variant::type eval ( const parse::node::type<Key> & node
                       , context<Key> & context
                       )
    {
      switch (node.flag)
        {
        case expr::parse::node::flag::value: return node.value;
        case expr::parse::node::flag::ref: return context.value (node.ref);
        case expr::parse::node::flag::unary:
          {
            const variant::type c (eval (*node.child0, context));

            return boost::apply_visitor ( token::function::unary (node.token)
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
              const variant::type l (eval (*node.child0, context));
              const variant::type r (eval (*node.child1, context));

              return boost::apply_visitor ( token::function::binary (node.token)
                                          , l
                                          , r
                                          );
            }
        case expr::parse::node::flag::ternary:
          throw std::runtime_error ("not implemented: ternary");
//           return token::function::ternary<double> ( node.token
//                                                   , eval (*node.child0, context)
//                                                   , eval (*node.child1, context)
//                                                   , eval (*node.child2, context)
//                                                   );
        default: throw parse::node::unknown();
        }
    }
  }
}

#endif
