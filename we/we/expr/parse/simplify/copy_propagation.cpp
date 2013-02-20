// bernd.loerwald@itwm.fraunhofer.de

#include <we/expr/parse/simplify/copy_propagation.hpp>

#include <we/expr/parse/simplify/expression_list.hpp>
#include <we/expr/parse/simplify/util.hpp>
#include <we/expr/token/assoc.hpp>
#include <we/expr/token/prop.hpp>

#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      namespace
      {
        typedef boost::unordered_map < key_type
                                     , key_type
                                     > propagation_map_type;

        void remove_parents_and_children_both
          (const key_type& var, propagation_map_type& map)
        {
          key_type current_parent (var);
          while (!current_parent.empty())
          {
            for ( propagation_map_type::iterator it (map.begin())
                , end (map.end())
                ; it != end
                ;
                )
            {
              if (it->first == current_parent || it->second == current_parent)
              {
                it = map.erase(it);
              }
              else
              {
                ++it;
              }
            }
            current_parent.pop_back();
          }

          for ( propagation_map_type::iterator it (map.begin()), end (map.end())
              ; it != end
              ;
              )
          {
            if ( util::begins_with (it->first, var)
               || util::begins_with (it->second, var)
               )
            {
              it = map.erase(it);
            }
            else
            {
              ++it;
            }
          }
        }

        class propagation : public boost::static_visitor<node::type>
        {
        public:
          propagation (propagation_map_type& propagation_map)
            : _propagation_map (propagation_map)
          { }

          node::type operator() (const value::type& v) const
          {
            return v;
          }

          node::type operator() (const key_type& k) const
          {
            key_type current_parent (k);
            key_type suffix;
            while (!current_parent.empty())
            {
              if (_propagation_map.count (current_parent))
              {
                key_type replacement (_propagation_map[current_parent]);
                replacement.insert ( replacement.end()
                                   , suffix.rbegin()
                                   , suffix.rend()
                                   );
                return replacement;
              }
              suffix.push_back (current_parent.back());
              current_parent.pop_back();
            }

            return k;
          }

          node::type operator() (node::unary_t& u) const
          {
            u.child = boost::apply_visitor (*this, u.child);
            return u;
          }

          node::type operator() (node::binary_t& b) const
          {
            if (token::is_define (b.token))
            {
              b.r = boost::apply_visitor (*this, b.r);

              key_type lhs (boost::get<key_type> (b.l));
              remove_parents_and_children_both (lhs, _propagation_map);
              if (const key_type* rhs = boost::get<key_type> (&b.r))
              {
                _propagation_map[lhs] = *rhs;
              }
            }
            else
            {
              if ( associativity::associativity (b.token)
                 == associativity::left
                 )
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

        private:
          propagation_map_type& _propagation_map;
        };
      }

      void copy_propagation (expression_list& list)
      {
        propagation_map_type propagation_map;

        for ( expression_list::nodes_type::iterator it (list.begin())
            , end (list.end())
            ; it != end
            ; ++it )
        {
          *it = boost::apply_visitor (propagation (propagation_map), *it);
        }
      }
    }
  }
}
