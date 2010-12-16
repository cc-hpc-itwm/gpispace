// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_CONTEXT_HPP
#define _EXPR_EVAL_CONTEXT_HPP

#include <we/expr/parse/node.hpp>

#include <iostream>
#include <stdexcept>

#include <we/type/value/container/container.hpp>
#include <we/type/value/container/bind.hpp>
#include <we/type/value/container/value.hpp>

namespace expr
{
  namespace eval
  {
    struct context
    {
    public:
      typedef expr::token::key_vec_t key_vec_t;

    private:
      typedef value::container::type container_t;
      container_t container;

    public:
      typedef container_t::const_iterator const_iterator;
      typedef container_t::iterator iterator;

      template<typename Path>
      value::type bind ( const std::string & key
                       , const Path & path
                       , const value::type & value
                       )
      {
        return value::container::bind<Path> (container, key, path, value);
      }

      value::type bind (const key_vec_t & key_vec, const value::type & value)
      {
        return value::container::bind (container, key_vec, value);
      }

      value::type bind (const std::string & key, const value::type & value)
      {
        return value::container::bind (container, key, value);
      }

      const value::type & value (const std::string & key) const
      {
        return value::container::value (container, key);
      }

      const value::type & value (const key_vec_t & key_vec) const
      {
        return value::container::value (container, key_vec);
      }

      value::type clear ()
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
      for ( context::const_iterator it (cntx.begin())
          ; it != cntx.end()
          ; ++it
          )
        s << it->first << " := " << it->second << std::endl;

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
