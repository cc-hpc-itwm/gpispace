// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_CONTEXT_HPP
#define _EXPR_EVAL_CONTEXT_HPP

#include <we/expr/parse/node.hpp>

#include <iostream>
#include <stdexcept>

#include <we/type/value/container/container.hpp>
#include <we/type/value/container/bind.hpp>
#include <we/type/value/container/value.hpp>
#include <we/type/value/container/show.hpp>

namespace expr
{
  namespace eval
  {
    struct context
    {
    public:
      typedef value::container::key_vec_t key_vec_t;

    private:
      typedef value::container::type container_t;
      container_t container;

    public:
      typedef container_t::const_iterator const_iterator;
      typedef container_t::iterator iterator;

      inline void bind (const key_vec_t & key_vec, const value::type & value)
      {
        value::container::bind (container, key_vec, value);
      }

      inline void bind (const std::string & key, const value::type & value)
      {
        value::container::bind (container, key, value);
      }

      inline const value::type &
      value (const std::string & key) const
      {
        return value::container::value (container, key);
      }

      inline const value::type &
      value (const key_vec_t & key_vec) const
      {
        return value::container::value (container, key_vec);
      }

      inline value::type clear ()
      {
        container.clear();
        return control();
      }

      const_iterator begin (void) const { return container.begin(); }
      const_iterator end (void) const { return container.end(); }
      std::size_t size (void) const { return container.size(); }

      friend std::ostream & operator << (std::ostream &, const context &);
    };

    inline std::ostream & operator << (std::ostream & s, const context & cntx)
    {
      value::container::show (s, cntx.container);

      return s;
    }

    inline parse::node::type
    refnode_value ( const context & context
                  , const context::key_vec_t & key_vec
                  )
    {
      return parse::node::type (context.value(key_vec));
    }

    inline parse::node::type
    refnode_name (const context::key_vec_t & key_vec)
    {
      return parse::node::type (key_vec);
    }
  }
}

#endif
