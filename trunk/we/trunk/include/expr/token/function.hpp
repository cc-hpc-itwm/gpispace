// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_TOKEN_FUNCTION_HPP
#define _EXPR_TOKEN_FUNCTION_HPP

#include <util/show.hpp>
#include <expr/token/type.hpp>
#include <expr/exception.hpp>

#include <math.h>

namespace expr
{
  namespace token
  {
    namespace function
    {
      template<typename T>
      static bool is_zero (const T & x)
      {
        return (fabs (x) < 1e-6) ? true : false;
      }

      template<typename T>
      static T unary (const type & token, const T & x)
      {
        switch (token)
          {
          case _not: return (is_zero (x)) ? 1 : 0;
          case neg: return -x;
          case abs: return is_zero (x) ? 0 : ((x < 0) ? (-x) : x);
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
          case _or: return int(l) | int(r);
          case _and: return int(l) & int(r);
          case lt: return is_zero (l-r) ? 0 : (l < r);
          case le: return is_zero (l-r) ? 1 : (l <= r);
          case gt: return is_zero (l-r) ? 0 : (l > r);
          case ge: return is_zero (l-r) ? 1 : (l >= r);
          case ne: return is_zero (l-r) ? 0 : 1;
          case eq: return is_zero (l-r) ? 1 : 0;
          case add: return l + r;
          case sub: return l - r;
          case mul: return l * r;
          case div: if (is_zero(r)) throw divide_by_zero(); return l / r;
          case mod: return int(l) % int(r);
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
