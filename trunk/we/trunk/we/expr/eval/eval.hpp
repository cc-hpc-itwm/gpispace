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
    namespace visitor
    {
      namespace node = expr::parse::node;

      class eval : public boost::static_visitor<value::type>
      {
      private:
        context & c;

      public:
        eval (context & _c) : c (_c) {}

        value::type operator () (const value::type & v) const
        {
          return v;
        }

        value::type operator () (const node::key_vec_t & key) const
        {
          return c.value (key);
        }

        value::type operator () (const node::unary_t & u) const
        {
          value::type c0 (boost::apply_visitor (*this, u.child));

          return boost::apply_visitor ( value::function::unary (u.token)
                                      , c0
                                      );
        }

        value::type operator () (const node::binary_t & b) const
        {
          if (is_define (b.token))
            {
              value::type c1 (boost::apply_visitor (*this, b.r));

              return c.bind (boost::get<node::key_vec_t>(b.l), c1);
            }
          else if (is_or (b.token))
            {
              value::type c0 (boost::apply_visitor (*this, b.l));

              if (!value::function::is_true (c0))
                {
                  value::type c1 (boost::apply_visitor (*this, b.r));

                  return boost::apply_visitor ( value::function::binary (b.token)
                                              , c0
                                              , c1
                                              );
                }
              else
                {
                  return c0;
                }
            }
          else if (is_and (b.token))
            {
              value::type c0 (boost::apply_visitor (*this, b.l));

              if (value::function::is_true (c0))
                {
                  value::type c1 (boost::apply_visitor (*this, b.r));

                  return boost::apply_visitor ( value::function::binary (b.token)
                                              , c0
                                              , c1
                                              );
                }
              else
                {
                  return c0;
                }
            }
          else
            {
              value::type c0 (boost::apply_visitor (*this, b.l));
              value::type c1 (boost::apply_visitor (*this, b.r));

              return boost::apply_visitor ( value::function::binary (b.token)
                                          , c0
                                          , c1
                                          );
            }

        }

        value::type operator () (const node::ternary_t & t) const
        {
          if (t.token == expr::token::_ite)
            {
              if ( value::function::is_true ( boost::apply_visitor ( *this
                                                                   , t.child0
                                                                   )
                                            )
                 )
                return boost::apply_visitor (*this, t.child1);
              else
                return boost::apply_visitor (*this, t.child2);
            }
          else
            throw expr::exception::strange ("ternary but not ite");
        }
      };
    }
  }
}

#endif
