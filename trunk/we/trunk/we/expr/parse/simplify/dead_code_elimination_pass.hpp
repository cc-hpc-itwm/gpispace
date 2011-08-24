// bernd.loerwald@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_SIMPLIFY_DEAD_CODE_ELIMINATION_PASS_HPP
#define _EXPR_PARSE_SIMPLIFY_DEAD_CODE_ELIMINATION_PASS_HPP 1

#include <boost/variant.hpp>

#include <we/expr/token/assoc.hpp>
#include <we/expr/parse/node.hpp>
#include <we/expr/parse/util/get_names.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      namespace detail
      {
        class dead_code_elimination_pass
          : public boost::static_visitor<node::type>
        {
        private:
          util::name_set_t* _referenced_names;
          mutable bool _has_assignment_somewhere;

        public:
          dead_code_elimination_pass (util::name_set_t * referenced_names)
            : _referenced_names (referenced_names)
            , _has_assignment_somewhere (false)
          {}

          node::type operator () (const value::type & t) const
          {
            return t;
          }

          node::type operator () (const node::key_vec_t & t) const
          {
            _referenced_names->insert (t);
            return t;
          }

          node::type operator () (node::unary_t & u) const
          {
            u.child = boost::apply_visitor (*this, u.child);
            return u;
          }

        private:
          static bool
          partially_match (const node::key_vec_t & lhs, const node::key_vec_t & rhs)
          {
            typedef node::key_vec_t::const_iterator key_iter;
            key_iter lhs_it (lhs.begin ()), lhs_end (lhs.end ());
            key_iter rhs_it (rhs.begin ()), rhs_end (rhs.end ());

            //! \note This is std::mismatch with an additional check for rhs_end.
            while (lhs_it != lhs_end && rhs_it != rhs_end)
            {
              if (*lhs_it != *rhs_it)
              {
                return false;
              }
              ++lhs_it;
              ++rhs_it;
            }
            return true;
          }

          bool variable_or_member_referenced (const node::key_vec_t & k) const
          {
            if (_referenced_names->count (k))
              return true;

            for ( util::name_set_t::const_iterator it (_referenced_names->begin ())
                  , end (_referenced_names->end ())
                ; it != end
                ; ++it
                )
            {
              if (partially_match (k, *it))
                return true;
            }

            return false;
          }

        public:

          node::type operator () (node::binary_t & b) const
          {
            if (token::is_define (b.token))
            {
              b.r = boost::apply_visitor (*this, b.r);

              //! \note If the left hand side never got referenced below, let  \
                        us be the right hand side. Else, mark, that there is   \
                        an assignment somewhere and stay unchanged.

              if (!variable_or_member_referenced (boost::get<node::key_vec_t> (b.l)))
              {
                return b.r;
              }

              _has_assignment_somewhere = true;
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

          bool had_assignment_somewhere () const
          {
            return _has_assignment_somewhere;
          }
        };
      }

      inline void
      dead_code_elimination_pass ( expression_list & list
                                 , const util::name_set_t & needed_bindings_ssa
                                 )
      {
        util::name_set_t referenced_names (needed_bindings_ssa);

        expression_list::node_stack_r_it_t it (list.rbegin());
        while (it != list.rend())
        {
          detail::dead_code_elimination_pass visitor (&referenced_names);
          *it = boost::apply_visitor (visitor, *it);

          if (!visitor.had_assignment_somewhere ())
          {
            it = expression_list::node_stack_r_it_t (list.erase (--it.base()));
          }
          else
          {
            ++it;
          }
        }
      }
    }
  }
}

#endif
