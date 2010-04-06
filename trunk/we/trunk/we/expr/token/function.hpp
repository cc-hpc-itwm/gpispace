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
      static inline bool is_zero (const double & x)
      {
        return (fabs (x) < 1e-6);
      }

      class visitor_is_zero : public boost::static_visitor<bool>
      {
      public:
        bool operator () (const long & x) const { return (x == 0); }
        bool operator () (const double & x) const { return is_zero (x); }

        bool operator () (const char & x) const
        {
          throw exception::eval::type_error
            ("is_zero ('" + util::show(x) + "')");
        }
        bool operator () (const std::string & x) const
        {
          throw exception::eval::type_error ("is_zero (\"" + x + "\")");
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
            case _sin: return sin (x);
            case _cos: return cos (x);
            case _sqrt: return sqrt (x);
            case _log: return log (x);
            case _floor:
            case _ceil:
            case _round:
            case _toint: return x;
            case _todouble: return double(x);
            default: throw exception::strange ("unary " + util::show(token));
            }
        }

        variant::type operator () (const double & x) const
        {
          static bool round_half_up (true);

          switch (token)
            {
            case _not: return is_zero(x) ? 1L : 0L;
            case neg: return -x;
            case abs: return (x < 0) ? (-x) : x;
            case _sin: return sin (x);
            case _cos: return cos (x);
            case _sqrt: return sqrt (x);
            case _log: return log (x);
            case _floor: return floor (x);
            case _ceil: return ceil(x);
            case _round:
              round_half_up = !round_half_up; 
              return round_half_up ? floor (x + 0.5) : ceil (x - 0.5);
            case _toint: return long(x);
            case _todouble: return x;
            default: throw exception::strange ("unary " + util::show(token));
            }
        }

        variant::type operator () (const char & x) const
        {
          throw exception::eval::type_error
            (util::show (token) + " ('" + util::show(x) + "')");
        }

        variant::type operator () (const std::string & x) const
        {
          throw exception::eval::type_error
            (util::show (token) + " (\"" + x + "\")");
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
            case add: return l + r;
            case sub: return l - r;
            case mul: return l * r;
            case div:
              if (r == 0) throw exception::eval::divide_by_zero();
              return double(l) / double(r);
            case divint:
              if (r == 0) throw exception::eval::divide_by_zero();
              return l / r;
            case modint:
            case mod:
              if (r == 0) throw exception::eval::divide_by_zero();
              return l % r;
            case _pow: return pow (l, r);
            case _powint:
              if (r < 0) throw exception::eval::negative_exponent();
              {
                long x (1);
                
                for (long i (0); i < r; ++i) x *= l;

                return x;
              }
            case min: return std::min (l,r);
            case max: return std::max (l,r);
            default: throw exception::strange ("binary " + util::show(token));
            }
        }

        variant::type operator () (const double & l, const double & r) const
        {
          switch (token)
            {
            case _or:
            case _and:
              throw exception::eval::type_error 
                (util::show (token) + " for value(s) of type double");
            case lt: return is_zero (l-r) ? 0L : (l < r);
            case le: return is_zero (l-r) ? 1L : (l < r);
            case gt: return is_zero (l-r) ? 0L : (l > r);
            case ge: return is_zero (l-r) ? 1L : (l > r);
            case ne: return is_zero (l-r) ? 0L : 1L;
            case eq: return is_zero (l-r) ? 1L : 0L;
            case add: return l + r;
            case sub: return l - r;
            case mul: return l * r;
            case div:
              if (is_zero (r)) throw exception::eval::divide_by_zero();
              return double(l) / double(r);
            case divint:
              if (is_zero (r)) throw exception::eval::divide_by_zero();
              return l / r;
            case modint:
            case mod:
              throw exception::eval::type_error 
                (util::show (token) + " for value(s) of type double");
            case _pow: return pow (l, r);
            case _powint:
              throw exception::eval::type_error 
                (util::show (token) + " for value(s) of type double");
            case min: return std::min (l,r);
            case max: return std::max (l,r);
            default: throw exception::strange ("binary " + util::show(token));
            }
        }

        variant::type operator () ( const std::string & l
                                  , const std::string & r
                                  ) const
        {
          switch (token)
            {
            case _or:
            case _and:
              throw exception::eval::type_error 
                (util::show (token) + " for value(s) of type string");
            case lt: return l < r ? 1L : 0L;
            case le: return l <= r ? 1L : 0L;
            case gt: return l > r ? 1L : 0L;
            case ge: return l >= r ? 1L : 0L;
            case ne: return l != r ? 1L : 0L;
            case eq: return l == r ? 1L : 0L;
            case add:
            case sub:
            case mul:
            case div:
            case divint:
            case modint:
            case mod:
            case _pow:
            case _powint:
              throw exception::eval::type_error 
                (util::show (token) + " for value(s) of type string");
            case min: return std::min (l,r);
            case max: return std::max (l,r);
            default: throw exception::strange ("binary " + util::show(token));
            }
        }

        variant::type operator () (const char & l, const char & r) const
        {
          switch (token)
            {
            case _or:
            case _and:
              throw exception::eval::type_error 
                (util::show (token) + " for value(s) of type string");
            case lt: return l < r ? 1L : 0L;
            case le: return l <= r ? 1L : 0L;
            case gt: return l > r ? 1L : 0L;
            case ge: return l >= r ? 1L : 0L;
            case ne: return l != r ? 1L : 0L;
            case eq: return l == r ? 1L : 0L;
            case add:
            case sub:
            case mul:
            case div:
            case divint:
            case modint:
            case mod:
            case _pow:
            case _powint:
              throw exception::eval::type_error 
                (util::show (token) + " for value(s) of type string");
            case min: return std::min (l,r);
            case max: return std::max (l,r);
            default: throw exception::strange ("binary " + util::show(token));
            }
        }

        template<typename T,typename U>
        variant::type operator () (const T & l, const U & r) const
        {
          throw exception::eval::type_error 
            (util::show (token) + " for value(s) of different types");
        }
      };

      static variant::type ternary ( const type & token 
                                   , const variant::type & a
                                   , const variant::type & b
                                   , const variant::type & c
                                   )
      {
        switch (token)
          {
          case _ite: return is_zero (a) ? c : b;
          default: throw exception::strange ("ternary but not ite");
          }
      }
    }
  }
}

#endif
