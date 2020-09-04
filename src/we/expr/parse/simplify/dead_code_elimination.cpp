// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <we/expr/parse/simplify/dead_code_elimination.hpp>

#include <we/expr/parse/simplify/util.hpp>
#include <we/expr/token/assoc.hpp>
#include <we/expr/token/prop.hpp>

#include <boost/variant.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      namespace
      {
        class elimination : public boost::static_visitor<node::type>
        {
        private:
          key_set_type& _referenced_names;
          mutable bool _has_assignment_somewhere;

        public:
          elimination (key_set_type& referenced_names)
          : _referenced_names (referenced_names)
          , _has_assignment_somewhere (false)
          {}

          node::type operator() (const pnet::type::value::value_type& t) const
          {
            return t;
          }

          node::type operator() (const key_type& key) const
          {
            _referenced_names.insert (key);
            return key;
          }

          node::type operator() (node::unary_t& u) const
          {
            u.child = boost::apply_visitor (*this, u.child);
            return u;
          }

        private:
          bool variable_or_member_referenced (const key_type& k) const
          {
            if (_referenced_names.count (k))
            {
              return true;
            }

            key_type current_parent (k);
            while (!current_parent.empty())
            {
              if (_referenced_names.count (current_parent))
              {
                return true;
              }
              current_parent.pop_back();
            }

            for ( key_set_type::const_iterator it (_referenced_names.begin())
                , end (_referenced_names.end())
                ; it != end
                ; ++it
                )
            {
              if (util::begins_with (*it, k))
              {
                return true;
              }
            }

            return false;
          }

        public:
          node::type operator() (node::binary_t& b) const
          {
            if (token::is_define (b.token))
            {
              key_type lhs (boost::get<key_type> (b.l));
              bool lhs_referenced (variable_or_member_referenced (lhs));

              _referenced_names.erase (lhs);

              for ( key_set_type::iterator it (_referenced_names.begin())
                  , end (_referenced_names.end())
                  ; it != end
                  ;
                  )
              {
                if (util::begins_with (*it, lhs))
                {
                  it = _referenced_names.erase (it);
                }
                else
                {
                  ++it;
                }
              }

              b.r = boost::apply_visitor (*this, b.r);

              if (!lhs_referenced)
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

          node::type operator() (node::ternary_t& t) const
          {
            t.child0 = boost::apply_visitor (*this, t.child0);
            t.child1 = boost::apply_visitor (*this, t.child1);
            t.child2 = boost::apply_visitor (*this, t.child2);
            return t;
          }

          bool had_assignment_somewhere() const
          {
            return _has_assignment_somewhere;
          }
        };
      }

      void dead_code_elimination
        (expression_list& list, const key_set_type& needed_bindings)
      {
        key_set_type referenced_names (needed_bindings);

        typedef expression_list::nodes_type::reverse_iterator reverse_iter;
        reverse_iter it (list.rbegin());

        while (it != list.rend())
        {
          elimination visitor (referenced_names);

          *it = boost::apply_visitor (visitor, *it);

          if (!visitor.had_assignment_somewhere())
          {
            it = reverse_iter (list.erase (--it.base()));
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
