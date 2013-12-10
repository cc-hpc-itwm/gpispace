// bernd.loerwald@itwm.fraunhofer.de

#include <we/expr/parse/simplify/constant_propagation.hpp>

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
                                     , pnet::type::value::value_type
                                     > propagation_map_type;

        void remove_parents_and_children_left
          (const key_type& var, propagation_map_type& map)
        {
          key_type current_parent (var);
          while (!current_parent.empty())
          {
            map.erase (current_parent);
            current_parent.pop_back();
          }

          for ( propagation_map_type::iterator it (map.begin()), end (map.end())
              ; it != end
              ;
              )
          {
            if (util::begins_with (it->first, var))
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

          node::type operator() (const pnet::type::value::value_type& v) const
          {
            return v;
          }

          node::type operator() (const key_type& k) const
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
              remove_parents_and_children_left (lhs, _propagation_map);
              //! \todo also propagate constant trees or fold them?
              if (const pnet::type::value::value_type* rhs = boost::get<pnet::type::value::value_type> (&b.r))
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

      void constant_propagation (expression_list& list)
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
