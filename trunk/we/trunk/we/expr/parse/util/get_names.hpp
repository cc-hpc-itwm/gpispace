// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_UTIL_GET_NAMES_HPP
#define _EXPR_PARSE_UTIL_GET_NAMES_HPP 1

#include <boost/unordered_set.hpp>

#include <we/expr/parse/node.hpp>

#include <we/type/value.hpp>

#include <boost/variant.hpp>

namespace expr
{
  namespace parse
  {
    namespace util
    {
      typedef boost::unordered_set<node::key_vec_t> name_set_t;

      namespace detail
      {
        class get_names : public boost::static_visitor<void>
        {
        private:
          name_set_t * _names;

        public:
          get_names (name_set_t * names) : _names (names) {}

          void operator () (const value::type &) const
          {}

          void operator () (const node::key_vec_t & key) const
          {
            _names->insert (key);
          }

          void operator () (const node::unary_t & u) const
          {
            boost::apply_visitor (*this, u.child);
          }

          void operator () (const node::binary_t & b) const
          {
            boost::apply_visitor (*this, b.l);
            boost::apply_visitor (*this, b.r);
          }

          void operator () (const node::ternary_t & t) const
          {
            boost::apply_visitor (*this, t.child0);
            boost::apply_visitor (*this, t.child1);
            boost::apply_visitor (*this, t.child2);
          }
        };
      }

      static name_set_t
      get_names (const expr::parse::node::type & nd)
      {
        name_set_t names;

        boost::apply_visitor
          ( detail::get_names (&names)
          , nd
          );

        return names;
      }
    }
  }
}

#endif
