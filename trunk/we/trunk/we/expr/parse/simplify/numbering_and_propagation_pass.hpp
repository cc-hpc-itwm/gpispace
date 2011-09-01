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
                                   , propagated_type
                                   > propagation_map_type;

      propagation_map_type _propagation_map;

      namespace detail
      {
        class numbering_and_propagation_pass
          : public boost::static_visitor<node::type>
        {
        private:
          tree_node_type& _ssa_tree;
          const line_type& _line;
          expression_list & _expression_list;

        public:
          numbering_and_propagation_pass ( tree_node_type& ssa_tree
                                         , const line_type & line
                                         , expression_list & list
                                         )
          : _ssa_tree (ssa_tree)
          , _line (line)
          , _expression_list (list)
          {}

          node::type operator () (const value::type & t) const
          {
            return t;
          }

        private:
          node::type do_propagation (const node::key_vec_t & key) const
          {
            propagation_map_type::const_iterator entry
                (_propagation_map.find (key));
            if (entry != _propagation_map.end())
              return entry->second;

            typedef node::key_vec_t::const_iterator key_const_iter;
            key_const_iter key_end (key.end ());

            typedef propagation_map_type::const_iterator map_iter;
            for ( map_iter it (_propagation_map.begin())
                , end (_propagation_map.end())
                ; it != end
                ; ++it
                )
            {
              const node::key_vec_t* rhs_value
                  (boost::get<node::key_vec_t> (&it->second));

              if (!rhs_value)
              {
                continue;
              }

              key_const_iter key_it (key.begin ());
              key_const_iter rhs_it (it->first.begin ());
              key_const_iter rhs_end (it->first.end ());

              while (key_it != key_end && rhs_it != rhs_end)
              {
                if (*key_it != *rhs_it)
                {
                  break;
                }
                ++key_it;
                ++rhs_it;
              }

              if (rhs_it != rhs_end || key_it == key_end)
              {
                continue;
              }

              key_type first_part (rhs_value->begin(), rhs_value->end());
              key_type second_part (key_it, key_end);

              for (key_type::iterator it (second_part.begin() + 1)
                  , end (second_part.end())
                  ; it != end && it + 1 != end
                  ; advance (it, 2)
                  )
              {
                *it = "*";
              }

              std::copy ( second_part.begin(), second_part.end()
                        , std::back_inserter (first_part));

              _ssa_tree.fill_asterics_with_latest (first_part);

              return first_part;
            }
            return key;
          }

          key_type
          create_temp_assignment ( const line_type & line
                                 , const key_type & name) const
          {
            key_type temp_name (name);
            temp_name.insert (temp_name.begin(), ".temp");
            temp_name.insert (temp_name.begin(), "0");

            propagation_map_type::const_iterator it
                (_propagation_map.find (temp_name));

            node::type rhs_node (name);

            //! \note Do not add another copy for this thing..
            if (it == _propagation_map.end ())
            {
              node::type assignment ( node::binary_t ( token::define
                                    , node::type (temp_name)
                                    , rhs_node));

              _expression_list.insert (line, assignment);
              _propagation_map[temp_name] = propagated_type (name);
            }
            return temp_name;
          }

          static bool
          is_before (const line_type & first, const line_type & second)
          {
            size_t fs (std::distance (first, second));
            size_t sf (std::distance (second, first));
            return fs > sf;
          }

        public:
          node::type operator () (const key_type & key) const
          {
            node::type propagated_node
                (do_propagation (_ssa_tree.get_current_name (key)));

            if (boost::get<value::type> (&propagated_node))
            {
              return propagated_node;
            }

            const node::key_vec_t propagated_key
                (boost::get<node::key_vec_t> (propagated_node));

            const line_type & key_line
                (_ssa_tree.get_line_of_next_write (propagated_key));
            if (key_line != line_type () && is_before (_line, key_line))
            {
                return create_temp_assignment (key_line, propagated_key);
            }
            return propagated_key;
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
              node::key_vec_t lhs_old (boost::get<node::key_vec_t> (b.l));

              b.r = boost::apply_visitor (*this, b.r);
              _ssa_tree.add_reference (lhs_old, _line);
              b.l = boost::apply_visitor (*this, b.l);

              node::key_vec_t lhs (boost::get<node::key_vec_t> (b.l));

              //! \note we only handle constant and copy propagation.
              if (const node::key_vec_t* rhs = boost::get<node::key_vec_t> (&b.r))
              {
                _propagation_map[lhs] = *rhs;
                _ssa_tree.latest_version_is_a_copy (lhs, *rhs);
              }
              else if (const value::type* rhs = boost::get<value::type> (&b.r))
              {
                _propagation_map[lhs] = *rhs;
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
            //! \todo Don't fail with control-flow-statements.
            if (t.token == token::_ite)
            {
              throw std::runtime_error("sorry, we can't handle simplifying "
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
      numbering_and_propagation_pass ( expression_list & list
                                     , tree_node_type& ssa_tree
                                     )
      {
        for ( expression_list::nodes_type::iterator it (list.begin())
            , end (list.end())
            ; it != end
            ; ++it )
        {
          *it = boost::apply_visitor
              ( detail::numbering_and_propagation_pass (ssa_tree, it, list)
              , *it
              );
        }
      }
    }
  }
}

#endif
