// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_VALUE_FUNCTION_HPP
#define _TYPE_VALUE_FUNCTION_HPP

#include <we/type/value.hpp>
#include <we/type/value/eq.hpp>
#include <we/type/literal.hpp>
#include <we/type/literal/function.hpp>

#include <we/expr/exception.hpp>

#include <fhg/util/show.hpp>

namespace value
{
  namespace function
  {
    namespace
    {
      class visitor_unary : public boost::static_visitor<type>
      {
      private:
        const expr::token::type& _token;

      public:
        visitor_unary (const expr::token::type& token)
          : _token (token)
        {}

        type operator() (literal::type& x) const
        {
          return literal::function::unary (_token, x);
        }

        type operator() (structured_t&) const
        {
          throw expr::exception::eval::type_error
            (fhg::util::show (_token) + " for a structured value");
        }
      };

      class visitor_binary : public boost::static_visitor<type>
      {
      private:
        const expr::token::type& _token;

      public:
        visitor_binary (const expr::token::type& token)
          : _token (token)
        {}

        type operator() (literal::type& x, literal::type& y) const
        {
          return literal::function::binary (_token, x, y);
        }

        type operator() (structured_t& x, structured_t& y) const
        {
          switch (_token)
          {
          case expr::token::eq: return value::eq (x, y);
          default:
            throw expr::exception::eval::type_error
              (fhg::util::show (_token) + " for structured values");
          }
        }

        template<typename A, typename B>
        type operator () (A&, B&) const
        {
          throw expr::exception::eval::type_error
            (fhg::util::show (_token) + " for literal and structured value");
        }
      };

      class visitor_is_true : public boost::static_visitor<bool>
      {
      public:
        bool operator() (const literal::type& x) const
        {
          return literal::function::is_true (x);
        }

        bool operator() (const structured_t &) const
        {
          throw expr::exception::eval::type_error
            ("is_true for a structured value");
        }
      };
    }

    value::type unary (const expr::token::type& t, value::type& v)
    {
      return boost::apply_visitor (visitor_unary (t), v);
    }

    value::type binary ( const expr::token::type& t
                       , value::type& l
                       , value::type& r
                       )
    {
      return boost::apply_visitor (visitor_binary (t), l, r);
    }

    bool is_true (const type & v)
    {
      return boost::apply_visitor (visitor_is_true(), v);
    }
  }
}

#endif
