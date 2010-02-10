// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_FUNCTION_HPP
#define _EXPR_TOKEN_FUNCTION_HPP

#include <util/show.hpp>
#include <expr/token/type.hpp>
#include <expr/exception.hpp>

namespace expr
{
  namespace token
  {
    namespace function
    {
      template<typename T>
      static T unary (const type & token, const T & x)
      {
        switch (token)
          {
          case _not: return !x;
          case neg: return -x;
          case abs: return (x < 0) ? (-x) : x;
          case fac:
            {
              T f (1);

              for (T i (1); i <= x; ++i)
                f *= i;

              return f;
            }
          default: throw std::runtime_error ("unary " + show(token));
          }
      }

      template<typename T>
      static T binary (const type & token, const T & l, const T & r)
      {
        switch (token)
          {
          case _or: return l | r;
          case _and: return l & r;
          case lt: return l < r;
          case le: return l <= r;
          case gt: return l > r;
          case ge: return l >= r;
          case ne: return l != r;
          case eq: return l == r;
          case add: return l + r;
          case sub: return l - r;
          case mul: return l * r;
          case div: if (r == 0) throw divide_by_zero(); return l / r;
          case mod: return l % r;
          case pow:
            {
              T p (1);

              for (T i (0); i < r; ++i)
                p *= l;
              
              return p; 
            }
          case min: return std::min (l,r);
          case max: return std::max (l,r);
          case com: return (unary<T>(fac, l)) / (unary<T>(fac, l-r));
          default: throw std::runtime_error ("binary " + show(token));
          }
      }
    }
  }
}

#endif
