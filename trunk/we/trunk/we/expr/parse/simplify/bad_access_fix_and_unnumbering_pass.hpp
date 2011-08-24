// bernd.loerwald@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_SIMPLIFY_BAD_ACCESS_FIX_AND_UNNUMBERING_PASS_HPP
#define _EXPR_PARSE_SIMPLIFY_BAD_ACCESS_FIX_AND_UNNUMBERING_PASS_HPP 1

#include <boost/variant.hpp>

#include <we/expr/token/assoc.hpp>
#include <we/expr/parse/node.hpp>
#include <we/expr/parse/util/get_names.hpp>
#include <we/expr/parse/simplify/ssa_tree.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      namespace detail
      {
        class bad_access_fix_and_unnumbering_pass
          : public boost::static_visitor<node::type>
        {
        private:
          tree_node_type& _ssa_tree;
          const line_type& _line;

        public:
          bad_access_fix_and_unnumbering_pass ( tree_node_type& ssa_tree
                                              , const line_type & line
                                              )
          : _ssa_tree (ssa_tree)
          , _line (line)
          {}

          node::type operator () (const value::type & t) const
          {
            return t;
          }

        private:
          static node::key_vec_t
          get_normal_name (node::key_vec_t ssa_name)
          {
            signed long i = ssa_name.size() - 1;
            while (i >= 0)
            {
              ssa_name.erase (ssa_name.begin() + i);
              i -= 2;
            }
            return ssa_name;
          }

        public:
          node::type operator () (const node::key_vec_t & key) const
          {
            return get_normal_name (key);
          }

          node::type operator () (node::unary_t & u) const
          {
            u.child = boost::apply_visitor (*this, u.child);
            return u;
          }

          node::type operator () (node::binary_t & b) const
          {
            if (token::is_define (b.token))
            {
              b.r = boost::apply_visitor (*this, b.r);
              //inc (_ssa_tree, boost::get<node::key_vec_t> (b.l), _line);
              b.l = boost::apply_visitor (*this, b.l);
            }
            else
            {
              if (associativity::associativity (b.token) == associativity::left)
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

          node::type operator () (node::ternary_t & t) const
          {
            t.child0 = boost::apply_visitor (*this, t.child0);
            t.child1 = boost::apply_visitor (*this, t.child1);
            t.child2 = boost::apply_visitor (*this, t.child2);
            return t;
          }
        };
      }

      static void
      bad_access_fix_and_unnumbering_pass ( expression_list & list
                                          , tree_node_type& ssa_tree
                                          )
      {
        for ( expression_list::node_stack_it_t it (list.begin()), end (list.end())
            ; it != end
            ; ++it )
        {
          *it = boost::apply_visitor
              ( detail::bad_access_fix_and_unnumbering_pass (ssa_tree, it)
              , *it
              );
        }
      }
    }
  }
}

#endif
