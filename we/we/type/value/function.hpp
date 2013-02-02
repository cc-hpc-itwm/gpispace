// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_VALUE_FUNCTION_HPP
#define _TYPE_VALUE_FUNCTION_HPP

#include <we/type/value.hpp>
#include <we/type/value/eq.hpp>
#include <we/type/literal.hpp>
#include <we/type/literal/function.hpp>

#include <fhg/util/show.hpp>

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
          (fhg::util::show (token) + " for a structured value");
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

      type operator () (structured_t & x, structured_t & y) const
      {
        switch (token)
          {
          case expr::token::eq: return value::visitor::eq()(x, y);
          default:
            throw expr::exception::eval::type_error
              (fhg::util::show (token) + " for structured values");
          }
      }

      template<typename A, typename B>
      type operator () (A &, B &) const
      {
        throw expr::exception::eval::type_error
          (fhg::util::show (token) + " for literal and structured value");
      }
    };
  }
}

#endif
