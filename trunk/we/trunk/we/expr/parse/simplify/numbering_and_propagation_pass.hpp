// bernd.loerwald@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_SIMPLIFY_NUMBERING_AND_PROPAGATION_PASS_HPP
#define _EXPR_PARSE_SIMPLIFY_NUMBERING_AND_PROPAGATION_PASS_HPP 1

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
      typedef boost::variant<value::type, node::key_vec_t> propagated_type;
      typedef boost::unordered_map < node::key_vec_t
                                   , propagated_type> propagation_map_t;

      propagation_map_t _propagation_map;

      namespace detail
      {
        class numbering_and_propagation_pass
          : public boost::static_visitor<node::type>
        {
        private:
          tree_node_type& _ssa_tree;
          const line_type& _line;

        public:
          numbering_and_propagation_pass ( tree_node_type& ssa_tree
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
          node::type do_propagation (const node::key_vec_t & key) const
          {
            propagation_map_t::const_iterator entry
                (_propagation_map.find (key));
            if (entry != _propagation_map.end())
            {
              return entry->second;
            }
            return key;
          }

        public:
          node::type operator () (const node::key_vec_t & key) const
          {
            return do_propagation (_ssa_tree.get_ssa_name (key));
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
              inc (_ssa_tree, boost::get<node::key_vec_t> (b.l), _line);
              b.l = boost::apply_visitor (*this, b.l);

              //! \note we only handle constant and copy propagation, not expression propagation.
              if (const node::key_vec_t* rhs = boost::get<node::key_vec_t> (&b.r))
              {
                _propagation_map[boost::get<node::key_vec_t> (b.l)] = *rhs;
              }
              else if (const value::type* rhs = boost::get<value::type> (&b.r))
              {
                _propagation_map[boost::get<node::key_vec_t> (b.l)] = *rhs;
              }
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
            if (t.token == token::_ite)
              //! \todo Don't fail with control-flow-statements.
              throw std::runtime_error("sorry, we can't handle simplifying expressions with control flow expressions right now.");
            t.child0 = boost::apply_visitor (*this, t.child0);
            t.child1 = boost::apply_visitor (*this, t.child1);
            t.child2 = boost::apply_visitor (*this, t.child2);
            return t;
          }
        };
      }

      static void
      numbering_and_propagation_pass ( expression_list & list
                                     , tree_node_type& ssa_tree
                                     )
      {
        for ( expression_list::node_stack_it_t it (list.begin()), end (list.end())
            ; it != end
            ; ++it )
        {
          *it = boost::apply_visitor
              ( detail::numbering_and_propagation_pass (ssa_tree, it)
              , *it
              );
        }
      }
    }
  }
}

#endif
