// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_FUNCTION_HPP
#define _EXPR_TOKEN_FUNCTION_HPP

#include <we/util/show.hpp>
#include <we/expr/token/type.hpp>
#include <we/expr/exception.hpp>

#include <we/expr/variant/variant.hpp>

#include <math.h>

namespace expr
{
  namespace token
  {
    namespace function
    {
      class visitor_is_zero : public boost::static_visitor<bool>
      {
      public:
        bool operator () (const long & x) const { return (x == 0); }
        bool operator () (const double & x) const { return (fabs (x) < 1e-6); }

        template<typename T>
        bool operator () (const T &) const
        {
          throw std::runtime_error ("STRANGE! is_zero for non-num");
        }
      };

      static bool is_zero (const variant::type & v)
      {
        static const visitor_is_zero z;

        return boost::apply_visitor (z, v);
      }

      class unary : public boost::static_visitor<variant::type>
      {
      private:
        const type & token;
      public:
        unary (const type & _token) : token (_token) {}

        variant::type operator () (const long & x) const
        {
          switch (token)
            {
            case _not: return (x == 0) ? 1L : 0L;
            case neg: return -x;
            case abs: return (x < 0) ? (-x) : x;
            case _sin: return sin (double(x));
            case _cos: return cos (double(x));
            case _sqrt: return sqrt (double(x));
            case _log: return log (double(x));
            case _floor:
            case _ceil:
            case _round:
              return x;
            default: throw std::runtime_error ("unary " + util::show(token));
            }
        }

        variant::type operator () (const double & x) const
        {
          switch (token)
            {
            case _not: return (fabs (x) < 1e-6) ? 1L : 0L;
            case neg: return -x;
            case abs: return (x < 0) ? (-x) : x;
            case _sin: return sin (x);
            case _cos: return cos (x);
            case _sqrt: return sqrt (x);
            case _log: return log (x);
            case _floor:
            case _ceil:
            case _round:
              return x;
            default: throw std::runtime_error ("unary " + util::show(token));
            }
        }

        template<typename T>
        variant::type operator () (const T & x) const
        {
          throw std::runtime_error ("not implemented: unary");
        }
      };

      class binary : public boost::static_visitor<variant::type>
      {
      private:
        const type & token;
      public:
        binary (const type & _token) : token (_token) {}

        variant::type operator () (const long & l, const long & r) const
        {
          switch (token)
            {
            case _or: return l | r;
            case _and: return l & r;
            case lt: return l < r ? 1L : 0L;
            case le: return l <= r ? 1L : 0L;
            case gt: return l > r ? 1L : 0L;
            case ge: return l >= r ? 1L : 0L;
            case ne: return l != r ? 1L : 0L;
            case eq: return l == r ? 1L : 0L;
            case add: return l + r ? 1L : 0L;
            case sub: return l - r ? 1L : 0L;
            case mul: return l * r ? 1L : 0L;
            case div:
            case divint:
              if (r == 0) throw divide_by_zero();
              return l / r;
            case modint:
            case mod:
              if (r == 0) throw divide_by_zero();
              return l % r;
            case _pow: return pow (l, r);
            case min: return std::min (l,r);
            case max: return std::max (l,r);
            default: throw std::runtime_error ("binary " + util::show(token));
            }
        }

        template<typename T, typename U>
        variant::type operator () (const T &, const U &) const
        {
          throw std::runtime_error ("not implemented: binary");
        }
      };
    }
  }
}

#endif
