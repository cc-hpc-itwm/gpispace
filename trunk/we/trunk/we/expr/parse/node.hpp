// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_NODE_HPP
#define _EXPR_PARSE_NODE_HPP

#include <we/expr/token/type.hpp>
#include <we/expr/token/prop.hpp>
#include <we/expr/token/tokenizer.hpp>
#include <we/expr/exception.hpp>

#include <we/type/value.hpp>

#include <stdexcept>
#include <iostream>
#include <vector>

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
        unknown () : std::runtime_error ("STRANGE! unknown node type") {};
      };

      namespace flag
      {
        enum flag
        { value
        , ref
        , unary
        , binary
        , ternary
        };
      };

      // WORK HERE: better type with union!?
      template<typename Key>
      struct type
      {
        typedef boost::shared_ptr<type> ptr_t;
        typedef std::vector<Key> key_vec_t;

        flag::flag flag;
        value::type value;
        key_vec_t ref;
        token::type token;
        ptr_t child0;
        ptr_t child1;
        ptr_t child2;

        type (const value::type & _value)
          : flag (flag::value)
          , value (_value)
          , ref ()
          , token ()
          , child0 ()
          , child1 ()
          , child2 ()
        {}

        type (const key_vec_t & _ref)
          : flag (flag::ref)
          , value ()
          , ref (_ref)
          , token ()
          , child0 ()
          , child1 ()
          , child2 ()
        {}

        type (const token::type & _token, const ptr_t _child0)
          : flag (flag::unary)
          , value ()
          , ref ()
          , token (_token)
          , child0 (_child0)
          , child1 ()
          , child2 ()
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
          , child2 ()
        {}

        type ( const token::type & _token
             , const ptr_t _child0
             , const ptr_t _child1
             , const ptr_t _child2
             )
          : flag (flag::ternary)
          , value ()
          , ref ()
          , token (_token)
          , child0 (_child0)
          , child1 (_child1)
          , child2 (_child2)
        {}
      };

      template<typename Key>
      std::ostream & operator << (std::ostream & s, const type<Key> & nd)
      {
        switch (nd.flag)
          {
          case flag::value: 
            return boost::apply_visitor ( value::visitor::show (s)
                                        , nd.value
                                        );
          case flag::ref: return s << "${" << token::show_key_vec (nd.ref) << "}";
          case flag::unary: return s << nd.token << "(" << *(nd.child0) << ")";
          case flag::binary:
            if (token::is_prefix (nd.token))
              return 
                s << nd.token 
                  << "(" << *(nd.child0) << ", " <<  *(nd.child1) << ")";
            else
              return 
                s << "(" << *(nd.child0) << nd.token << *(nd.child1) << ")";
          case flag::ternary:
            if (nd.token != token::_ite)
              throw exception::strange ("ternary but not _ite!?");

            return s << "(" << token::_if << *(nd.child0)
                            << token::_then << *(nd.child1)
                            << token::_else << *(nd.child2)
                            << token::_endif
                     << ")";
          default: throw unknown();
          }
      }

      template<typename Key>
      const value::type & get (const type<Key> & node)
      {
        if (node.flag != flag::value)
          throw exception::eval::type_error ("get: node is not an value");

        return node.value;
      }
    }
  }
}

#endif
