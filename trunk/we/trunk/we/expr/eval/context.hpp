// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_CONTEXT_HPP
#define _EXPR_EVAL_CONTEXT_HPP

#include <we/expr/parse/node.hpp>

#include <we/util/show.hpp>

#include <stdexcept>

#include <boost/unordered_map.hpp>

namespace expr
{
  namespace eval
  {
    template<typename Key>
    class missing_binding : public std::runtime_error
    {
    public:
      explicit missing_binding (const Key & key)
        : std::runtime_error ("missing binding for: ${" + show(key) + "}") {};
    };

    template<typename Key, typename Value>
    struct context
    {
    private:
      typedef boost::unordered_map<Key,Value> container_t;
      container_t container;
    public:
      typedef typename container_t::const_iterator const_iterator;

      Value bind (const Key & key, const Value & value)
      {
        container[key] = value; return value;
      }
      const Value & value (const Key & key) const
      {
        const const_iterator it (container.find (key));

        if (it == container.end())
          throw missing_binding<Key> (key);
        else
          return it->second;
      }
      void clear () { container.clear(); }

      const_iterator begin (void) const { return container.begin(); }
      const_iterator end (void) const { return container.end(); }
    };

    template<typename Key, typename Value>
    parse::node::type<Key,Value>
    refnode_value ( const context<Key,Value> & context
                  , const Key & key
                  )
    {
      return parse::node::type<Key,Value>(context.value(key));
    }

    template<typename Key,typename Value>
    parse::node::type<Key,Value> refnode_name (const Key & key) 
    {
      return parse::node::type<Key,Value>(key);
    }
  }
}

#endif
