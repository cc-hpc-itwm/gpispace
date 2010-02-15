// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_CONTEXT_HPP
#define _EXPR_EVAL_CONTEXT_HPP

#include <we/expr/parse/node.hpp>


#include <string>
#include <stdexcept>

#include <boost/unordered_map.hpp>

namespace expr
{
  namespace eval
  {
    class missing_binding : public std::runtime_error
    {
    public:
      explicit missing_binding (const std::string & name)
        : std::runtime_error ("missing binding for: ${" + name + "}") {};
    };
    
    template<typename T>
    struct context
    {
    private:
      boost::unordered_map<std::string,T> container;
    public:
      void bind (const std::string & name, const T & value)
      {
        container[name] = value;
      }
      const T & value (const std::string & name) const
      {
        typename boost::unordered_map<std::string,T>::const_iterator
          it (container.find (name));

        if (it == container.end())
          throw missing_binding (name);
        else
          return it->second;
      }
      void clear () { container.clear(); }
    };

    template<typename T>
    parse::node::type<T> refnode_value ( const eval::context<T> & context
                                       , const std::string & name
                                       )
    {
      return parse::node::type<T>(context.value(name));
    }

    template<typename T>
    parse::node::type<T> refnode_name (const std::string & name) 
    {
      return parse::node::type<T>(name);
    }
  }
}

#endif
