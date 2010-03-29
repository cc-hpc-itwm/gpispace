// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_NODE_HPP
#define _EXPR_PARSE_NODE_HPP

#include <we/expr/token/type.hpp>
#include <we/expr/token/prop.hpp>

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

      namespace flag
      {
        enum flag
        { value
        , ref
        , unary
        , binary
        };
      };

      // WORK HERE: better type with union!?
      template<typename Key, typename Value>
      struct type
      {
        typedef boost::shared_ptr<type> ptr_t;

        const flag::flag flag;
        const Value value;
        const Key ref;
        const token::type token;
        const ptr_t child0;
        const ptr_t child1;

        type (const Value & _value)
          : flag (flag::value)
          , value (_value)
          , ref ()
          , token ()
          , child0 ()
          , child1 ()
        {}

        type (const Key & _ref)
          : flag (flag::ref)
          , value ()
          , ref (_ref)
          , token ()
          , child0 ()
          , child1 ()
        {}

        type (const token::type & _token, const ptr_t _child0)
          : flag (flag::unary)
          , value ()
          , ref ()
          , token (_token)
          , child0 (_child0)
          , child1 ()
        {}

        type ( const token::type & _token
             , const ptr_t _child0
             , const ptr_t _child1
             )
          : flag (flag::binary)
          , value ()
          , ref ()
          , token (_token)
          , child0 (_child0)
          , child1 (_child1)
        {}

        bool is_value (void) const { return flag == flag::value; }
        bool is_ref (void) const { return flag == flag::ref; }
        bool is_unary (void) const { return flag == flag::unary; }
        bool is_binary (void) const { return flag == flag::binary; }
      };

      template<typename Key, typename Value>
      std::ostream & operator << (std::ostream & s, const type<Key,Value> & nd)
      {
        if (nd.is_value())
          return s << nd.value;

        if (nd.is_ref())
          return s << "${" << nd.ref << "}";

        if (nd.is_unary())
          return s << nd.token << "(" << *(nd.child0) << ")";

        if (nd.is_binary())
          {
            if (token::is_prefix (nd.token))
              return s << nd.token 
                       << "(" << *(nd.child0) << ", " <<  *(nd.child1) << ")";

            return s << "(" << *(nd.child0) << nd.token << *(nd.child1) << ")";
          }

        throw unknown();
      }

      template<typename Key, typename Value>
      const Value & get (const type<Key,Value> & node)
      {
        if (!node.is_value())
          throw std::runtime_error ("node is not an value");

        return node.value;
      }
    }
  }
}

#endif
