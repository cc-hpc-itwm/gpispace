// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_CONTEXT_HPP
#define _EXPR_EVAL_CONTEXT_HPP

#include <we/expr/parse/node.hpp>

#include <we/type/value.hpp>

#include <we/util/show.hpp>

#include <iostream>
#include <stdexcept>

#include <boost/unordered_map.hpp>

namespace expr
{
  namespace exception
  {
    namespace eval
    {
      template<typename Key>
      class missing_binding : public std::runtime_error
      {
      public:
        explicit missing_binding (const Key & key)
          : std::runtime_error 
            ("missing binding for: ${" + util::show(key) + "}") 
        {};
      };
    }
  }

  namespace eval
  {
    template<typename Key>
    struct context
    {
    private:
      typedef boost::unordered_map<Key,value::type> container_t;
      container_t container;
    public:
      typedef typename container_t::const_iterator const_iterator;

      value::type bind (const Key & key, const value::type & value)
      {
        container[key] = value; return value;
      }
      const value::type & value (const Key & key) const
      {
        const const_iterator it (container.find (key));

        if (it == container.end())
          throw exception::eval::missing_binding<Key> (key);
        else
          return it->second;
      }
      void clear () { container.clear(); }

      const_iterator begin (void) const { return container.begin(); }
      const_iterator end (void) const { return container.end(); }
      std::size_t size (void) const { return container.size(); }

      template<typename K>
      friend std::ostream & operator << (std::ostream &, const context<K> &);
    };

    template<typename K>
    std::ostream & operator << (std::ostream & s, const context<K> & cntx)
    {
      for ( typename context<K>::const_iterator it (cntx.begin())
          ; it != cntx.end()
          ; ++it
          )
        s << it->first << " := " << it->second << std::endl;

      return s;
    }

    template<typename Key>
    parse::node::type<Key>
    refnode_value ( const context<Key> & context
                  , const Key & key
                  )
    {
      return parse::node::type<Key>(context.value(key));
    }

    template<typename Key>
    parse::node::type<Key> refnode_name (const Key & key) 
    {
      return parse::node::type<Key>(key);
    }
  }
}

#endif
