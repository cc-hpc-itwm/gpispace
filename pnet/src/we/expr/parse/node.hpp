// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_NODE_HPP
#define _EXPR_PARSE_NODE_HPP

#include <we/expr/token/type.hpp>
#include <we/expr/token/tokenizer.hpp>

#include <we2/type/value.hpp>

#include <boost/variant.hpp>

namespace expr
{
  namespace parse
  {
    namespace node
    {
      struct unary_t;
      struct binary_t;
      struct ternary_t;

      typedef boost::variant < pnet::type::value::value_type
                             , std::list<std::string>
                             , boost::recursive_wrapper<unary_t>
                             , boost::recursive_wrapper<binary_t>
                             , boost::recursive_wrapper<ternary_t>
                             > type;

      std::ostream & operator << (std::ostream &, const type&);
      const pnet::type::value::value_type& get (const type&);
      bool is_value (const type&);
      bool is_ref (const type&);
      void rename (type&, const std::string& from, const std::string& to);

      struct unary_t
      {
        token::type token;
        type child;

        unary_t (const token::type& token, const type& child);
      };

      struct binary_t
      {
        token::type token;
        type l;
        type r;

        binary_t (const token::type& token, const type& l, const type& r);
      };

      struct ternary_t
      {
        token::type token;
        type child0;
        type child1;
        type child2;

        ternary_t ( const token::type& token
                  , const type& child0
                  , const type& child1
                  , const type& child2
                  );
      };
    }
  }
}

#endif
