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

      // ******************************************************************* //

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

              c.bind (boost::get<node::key_vec_t>(b.l), c1);

              return c1;
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
              if (associativity::associativity (b.token) == associativity::left)
                {
                  value::type c0 (boost::apply_visitor (*this, b.l));
                  value::type c1 (boost::apply_visitor (*this, b.r));

                  return boost::apply_visitor
                    (value::function::binary (b.token), c0, c1);
                }
              else
                {
                  value::type c1 (boost::apply_visitor (*this, b.r));
                  value::type c0 (boost::apply_visitor (*this, b.l));

                  return boost::apply_visitor
                    (value::function::binary (b.token), c0, c1);
                }
            }

        }

        value::type operator () (const node::ternary_t & t) const
        {
          switch (t.token)
            {
            case expr::token::_ite:
              if ( value::function::is_true ( boost::apply_visitor ( *this
                                                                   , t.child0
                                                                   )
                                            )
                 )
                {
                  return boost::apply_visitor (*this, t.child1);
                }
              else
                {
                  return boost::apply_visitor (*this, t.child2);
                }
              break;
            case expr::token::_map_assign:
              {
                value::type c0 (boost::apply_visitor (*this, t.child0));
                value::type c1 (boost::apply_visitor (*this, t.child1));
                value::type c2 (boost::apply_visitor (*this, t.child2));

                try
                  {
                    literal::map_type & m
                      (value::get_ref<literal::map_type> (c0));
                    long & k (value::get_ref<long> (c1));
                    long & v (value::get_ref<long> (c2));

                    m[k] = v;

                    return m;
                  }
                catch (...)
                  {
                    throw expr::exception::eval::type_error
                      ("map_assign (" + fhg::util::show (c0));
                  }
              }
              break;
            default:
              throw expr::exception::strange ("unknown ternary token");
            }
        }
      };
    }
  }
}

#endif
