// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_NODE_HPP
#define _EXPR_PARSE_NODE_HPP

#include <expr/token/type.hpp>
#include <expr/token/prop.hpp>

#include <string>
#include <stdexcept>
#include <iostream>

#include <boost/shared_ptr.hpp>

namespace expr
{
  namespace parse
  {
    namespace node
    {
      class unknown : public std::runtime_error
      {
      public:
        unknown () : std::runtime_error ("STRANGE: unknown node type") {};
      };

      // WORK HERE: better type with union!?
      template<typename T>
      struct type
      {
        typedef boost::shared_ptr<type> ptr_t;

        const bool is_value;
        const T value;

        const bool is_refname;
        const std::string refname;

        const bool is_unary;
        const bool is_binary;
        const token::type token;
        const ptr_t child0;
        const ptr_t child1;

        type (const T & _value) 
          : is_value (true)
          , value (_value)
          , is_refname (false)
          , refname ()
          , is_unary (false)
          , is_binary (false)
          , token ()
          , child0 ()
          , child1 ()
        {}

        type (const std::string & _refname)
          : is_value (false)
          , value ()
          , is_refname (true)
          , refname (_refname)
          , is_unary (false)
          , is_binary (false)
          , token ()
          , child0 ()
          , child1 ()
        {}

        type (const token::type & _token, const ptr_t _child0)
          : is_value (false)
          , value ()
          , is_refname (false)
          , refname ()
          , is_unary (true)
          , is_binary (false)
          , token (_token)
          , child0 (_child0)
          , child1 ()
        {}

        type ( const token::type & _token
             , const ptr_t _child0
             , const ptr_t _child1
             )
          : is_value (false)
          , value ()
          , is_refname (false)
          , refname ()
          , is_unary (false)
          , is_binary (true)
          , token (_token)
          , child0 (_child0)
          , child1 (_child1)
        {}
      };

      template<typename T>
      std::ostream & operator << (std::ostream & s, const type<T> & nd)
      {
        if (nd.is_value)
          return s << nd.value;

        if (nd.is_refname)
          return s << "${" << nd.refname << "}";

        if (nd.is_unary)
          return s << nd.token << "(" << *(nd.child0) << ")";

        if (nd.is_binary)
          {
            if (token::is_prefix (nd.token))
              return s << nd.token 
                       << "(" << *(nd.child0) << ", " <<  *(nd.child1) << ")";

            return s << "(" << *(nd.child0) << nd.token << *(nd.child1) << ")";
          }

        throw unknown();
      }

      template<typename T>
      const T & get (const type<T> & node)
      {
        if (!node.is_value)
          throw std::runtime_error ("node is not an value");

        return node.value;
      }
    }
  }
}

#endif
