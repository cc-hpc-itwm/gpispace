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

      class visitor_is_true : public boost::static_visitor<bool>
      {
      public:
        bool operator () (const bool & x) const
        {
          return x ? true : false;
        }

        template<typename T>
        bool operator () (const T &) const
        {
          throw exception::eval::type_error
            ("is_true for something that is not of type bool");
        }
      };

      static bool is_true (const variant::type & v)
      {
        static const visitor_is_true vt;

        return boost::apply_visitor (vt, v);
      }

      class unary : public boost::static_visitor<variant::type>
      {
      private:
        const type & token;
      public:
        unary (const type & _token) : token (_token) {}

        variant::type operator () (const control &) const
        {
          throw exception::eval::type_error
            (util::show (token) + " (control token)");
        }

        variant::type operator () (const bool & x) const
        {
          switch (token)
            {
            case _not: return x ? false : true;
            case neg:
            case abs:
            case _sin:
            case _cos:
            case _sqrt:
            case _log:
            case _floor:
            case _ceil:
            case _round:
              throw exception::eval::type_error
                (util::show (token) + " (" + util::show(x) + ")");
            case _tolong: return x ? 1L : 0L;
            case _todouble: return x ? 1.0 : 0.0;
            default: throw exception::strange ("unary " + util::show(token));
            }
        }

        variant::type operator () (const long & x) const
        {
          switch (token)
            {
            case _not: return (x == 0) ? true : false;
            case neg: return -x;
            case abs: return (x < 0) ? (-x) : x;
            case _sin: return sin (x);
            case _cos: return cos (x);
            case _sqrt: return sqrt (x);
            case _log: return log (x);
            case _floor:
            case _ceil:
            case _round:
            case _tolong: return x;
            case _todouble: return double(x);
            default: throw exception::strange ("unary " + util::show(token));
            }
        }

        variant::type operator () (const double & x) const
        {
          static bool round_half_up (true);

          switch (token)
            {
            case _not: return is_zero(x) ? true : false;
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
            case _tolong: return long(x);
            case _todouble: return x;
            default: throw exception::strange ("unary " + util::show(token));
            }
        }

        variant::type operator () (const char & x) const
        {
          switch (token)
          {
            case _len: return (long)(1);
            default:
              throw exception::eval::type_error
                (util::show (token) + " ('" + util::show(x) + "')");
          }
        }

        variant::type operator () (const std::string & x) const
        {
          switch (token)
          {
            case _len: return (long)(x.size());
            default:
              throw exception::eval::type_error
                (util::show (token) + " ('" + util::show(x) + "')");
          }
        }
      };

      class binary : public boost::static_visitor<variant::type>
      {
      private:
        const type & token;
      public:
        binary (const type & _token) : token (_token) {}

        variant::type operator () (const control &, const control &) const
        {
          throw exception::eval::type_error 
            (util::show (token) + " for control token");
        }

        variant::type operator () (const bool & l, const bool & r) const
        {
          switch (token)
            {
            case _or: return l || r;
            case _and: return l && r;
            case lt: return l < r ? true : false;
            case le: return l <= r ? true : false;
            case gt: return l > r ? true : false;
            case ge: return l >= r ? true : false;
            case ne: return l != r ? true : false;
            case eq: return l == r ? true : false;
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
                (util::show (token) + " for values of type bool");
            case min: return std::min (l,r);
            case max: return std::max (l,r);
            default: throw exception::strange ("binary " + util::show(token));
            }
        }

        variant::type operator () (const long & l, const long & r) const
        {
          switch (token)
            {
            case _or: return l | r;
            case _and: return l & r;
            case lt: return l < r ? true : false;
            case le: return l <= r ? true : false;
            case gt: return l > r ? true : false;
            case ge: return l >= r ? true : false;
            case ne: return l != r ? true : false;
            case eq: return l == r ? true : false;
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
                (util::show (token) + " for values of type double");
            case lt: return is_zero (l-r) ? false : (l < r ? true : false);
            case le: return is_zero (l-r) ? true : (l < r ? true : false);
            case gt: return is_zero (l-r) ? false : (l > r ? true : false);
            case ge: return is_zero (l-r) ? true : (l > r ? true : false);
            case ne: return is_zero (l-r) ? false : true;
            case eq: return is_zero (l-r) ? true : false;
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
                (util::show (token) + " for values of type double");
            case _pow: return pow (l, r);
            case _powint:
              throw exception::eval::type_error 
                (util::show (token) + " for values of type double");
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
                (util::show (token) + " for values of type string");
            case lt: return l < r ? true : false;
            case le: return l <= r ? true : false;
            case gt: return l > r ? true : false;
            case ge: return l >= r ? true : false;
            case ne: return l != r ? true : false;
            case eq: return l == r ? true : false;
            case add: { std::string s; s+= l; s += r; return s; }
            case sub:
            case mul:
            case div:
            case divint:
            case modint:
            case mod:
            case _pow:
            case _powint:
              throw exception::eval::type_error 
                (util::show (token) + " for values of type string");
            case min: return std::min (l,r);
            case max: return std::max (l,r);
            default: throw exception::strange ("binary " + util::show(token));
            }
        }

        variant::type operator () ( const std::string & l
                                  , const long & r
                                  ) const
        {
          switch (token)
            {
            case _substr:
              return l.substr(0, r);
            case _or:
            case _and:
            case lt:
            case le:
            case gt:
            case ge:
            case ne:
            case eq:
            case add:
            case sub:
            case mul:
            case div:
            case divint:
            case modint:
            case mod:
            case _pow:
            case min:
            case max:
            case _powint:
              throw exception::eval::type_error 
                (util::show (token) + " for value of type string");
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
                (util::show (token) + " for values of type char");
            case lt: return l < r ? true : false;
            case le: return l <= r ? true : false;
            case gt: return l > r ? true : false;
            case ge: return l >= r ? true : false;
            case ne: return l != r ? true : false;
            case eq: return l == r ? true : false;
            case add: { std::string s; s+= l; s += r; return s; }
            case sub:
            case mul:
            case div:
            case divint:
            case modint:
            case mod:
            case _pow:
            case _powint:
              throw exception::eval::type_error 
                (util::show (token) + " for values of type char");
            case min: return std::min (l,r);
            case max: return std::max (l,r);
            default: throw exception::strange ("binary " + util::show(token));
            }
        }

        template<typename T,typename U>
        variant::type operator () (const T &, const U &) const
        {
          throw exception::eval::type_error 
            (util::show (token) + " for values of different types");
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
          case _ite: return is_true (a) ? b : c;
          default: throw exception::strange ("ternary but not ite");
          }
      }
    }
  }
}

#endif
