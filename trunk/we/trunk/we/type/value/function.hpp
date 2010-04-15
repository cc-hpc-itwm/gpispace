// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_VALUE_FUNCTION_HPP
#define _TYPE_VALUE_FUNCTION_HPP

#include <we/type/value.hpp>
#include <we/type/literal.hpp>
#include <we/type/literal/function.hpp>

namespace value
{
  namespace function
  {
    namespace visitor
    {
      class is_true : public boost::static_visitor<bool>
      {
      public:
        bool operator () (const literal::type & x) const
        {
          return literal::function::is_true (x);
        }

        bool operator () (const structured_t &) const
        {
          throw expr::exception::eval::type_error
            ("is_true for a structured value");
        }
      };
    }

    static bool is_true (const type & v)
    {
      return boost::apply_visitor (visitor::is_true(), v);
    }

    class unary : public boost::static_visitor<type>
    {
    private:
      const expr::token::type & token;

    public:
      unary (const expr::token::type & _token) : token (_token) {}

      type operator () (literal::type & x) const
      {
        return boost::apply_visitor (literal::function::unary (token), x);
      }

      type operator () (structured_t &) const
      {
        throw expr::exception::eval::type_error
          (util::show (token) + " for a structured value");
      }
    };

    class binary : public boost::static_visitor<type>
    {
    private:
      const expr::token::type & token;

    public:
      binary (const expr::token::type & _token) : token (_token) {}

      type operator () (literal::type & x, literal::type & y) const
      {
        return boost::apply_visitor (literal::function::binary (token), x, y);
      }

      template<typename A, typename B>
      type operator () (A &, B &) const
      {
        throw expr::exception::eval::type_error
          (util::show (token) + " for structured value(s)");
      }
    };

    static type ternary ( const expr::token::type & token 
                        , const type & a
                        , const type & b
                        , const type & c
                        )
    {
      switch (token)
        {
        case expr::token::_ite: return is_true (a) ? b : c;
        default: throw expr::exception::strange ("ternary but not ite");
        }
    }
  }
}

#endif
