// bernd.loerwald@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_SIMPLIFY_CONSTANT_PROPAGATION_HPP
#define _EXPR_PARSE_SIMPLIFY_CONSTANT_PROPAGATION_HPP 1

#include <we/expr/parse/simplify/util.hpp>
#include <we/expr/parse/simplify/expression_list.hpp>

#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      namespace detail
      {
        class constant_propagation : public boost::static_visitor<node::type>
        {
          public:
            typedef boost::unordered_map < key_type
                                         , value::type
                                         > propagation_map_type;

          private:
            propagation_map_type & _propagation_map;

          public:
            constant_propagation (propagation_map_type & propagation_map)
            : _propagation_map (propagation_map)
            {}

            node::type operator() (const value::type & v) const
            {
              return v;
            }

            node::type operator() (const key_type & k) const
            {
              if (_propagation_map.count (k))
              {
                return _propagation_map[k];
              }
              else
              {
                return k;
              }
            }

            node::type operator() (node::unary_t & u) const
            {
              u.child = boost::apply_visitor (*this, u.child);
              return u;
            }

            node::type operator() (node::binary_t & b) const
            {
              if (token::is_define (b.token))
              {
                b.r = boost::apply_visitor (*this, b.r);

                key_type lhs (boost::get<key_type> (b.l));
                util::remove_parents_and_children_left (lhs, _propagation_map);
                //! \todo also propagate constant trees or fold them?
                if (const value::type* rhs = boost::get<value::type> (&b.r))
                {
                  _propagation_map[lhs] = *rhs;
                }
              }
              else
              {
                if (associativity::associativity (b.token)
                                                         == associativity::left)
                {
                  b.l = boost::apply_visitor (*this, b.l);
                  b.r = boost::apply_visitor (*this, b.r);
                }
                else
                {
                  b.r = boost::apply_visitor (*this, b.r);
                  b.l = boost::apply_visitor (*this, b.l);
                }
              }
              return b;
            }

            node::type operator() (node::ternary_t & t) const
            {
              //! \todo Don't fail with control-flow-statements.
              if (t.token == token::_ite)
              {
                throw std::runtime_error ("sorry, we can't handle simplifying "
                                          "expressions with control flow "
                                          "expressions right now.");
              }
              t.child0 = boost::apply_visitor (*this, t.child0);
              t.child1 = boost::apply_visitor (*this, t.child1);
              t.child2 = boost::apply_visitor (*this, t.child2);
              return t;
            }
        };
      }

      inline void
      constant_propagation (expression_list & list)
      {
        detail::constant_propagation::propagation_map_type propagation_map;

        for ( expression_list::nodes_type::iterator it (list.begin())
            , end (list.end())
            ; it != end
            ; ++it )
        {
          *it = boost::apply_visitor
              ( detail::constant_propagation (propagation_map)
              , *it
              );
        }
      }
    }
  }
}

#endif
