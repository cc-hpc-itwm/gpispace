// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_LITERAL_FUNCTION_HPP
#define _TYPE_LITERAL_FUNCTION_HPP

#include <we/util/show.hpp>
#include <we/expr/token/type.hpp>
#include <we/expr/exception.hpp>

#include <we/type/literal.hpp>
#include <we/type/bitsetofint.hpp>

#include <math.h>

namespace literal
{
  namespace function
  {
    static inline bool is_zero (const double & x)
    {
      return (fabs (x) < 1e-6);
    }

    namespace visitor
    {
      class is_true : public boost::static_visitor<bool>
      {
      public:
        bool operator () (const bool & x) const
        {
          return x ? true : false;
        }

        template<typename T>
        bool operator () (const T &) const
        {
          throw expr::exception::eval::type_error
            ("is_true for something that is not of type bool");
        }
      };
    }

    static bool is_true (const literal::type & v)
    {
      return boost::apply_visitor (visitor::is_true(), v);
    }

    class unary : public boost::static_visitor<literal::type>
    {
    private:
      const expr::token::type & token;
    public:
      unary (const expr::token::type & _token) : token (_token) {}

      literal::type operator () (control &) const
      {
        throw expr::exception::eval::type_error
          (util::show (token) + " (control token)");
      }

      literal::type operator () (bitsetofint::type &) const
      {
        throw expr::exception::eval::type_error
          (util::show (token) + " (bitset)");
      }

      literal::type operator () (bool & x) const
      {
        switch (token)
          {
          case expr::token::_not: return x ? false : true;
          case expr::token::neg:
          case expr::token::abs:
          case expr::token::_sin:
          case expr::token::_cos:
          case expr::token::_sqrt:
          case expr::token::_log:
          case expr::token::_floor:
          case expr::token::_ceil:
          case expr::token::_round:
            throw expr::exception::eval::type_error
              (util::show (token) + " (" + util::show(x) + ")");
          case expr::token::_tolong: return x ? 1L : 0L;
          case expr::token::_todouble: return x ? 1.0 : 0.0;
          default: throw expr::exception::strange ("unary " + util::show(token));
          }
      }

      literal::type operator () (long & x) const
      {
        switch (token)
          {
          case expr::token::_not: return (x == 0) ? true : false;
          case expr::token::neg: return -x;
          case expr::token::abs: return (x < 0) ? (-x) : x;
          case expr::token::_sin: return sin (x);
          case expr::token::_cos: return cos (x);
          case expr::token::_sqrt: return sqrt (x);
          case expr::token::_log: return log (x);
          case expr::token::_floor:
          case expr::token::_ceil:
          case expr::token::_round:
          case expr::token::_tolong: return x;
          case expr::token::_todouble: return double(x);
          default: throw expr::exception::strange ("unary " + util::show(token));
          }
      }

      literal::type operator () (double & x) const
      {
        static bool round_half_up (true);

        switch (token)
          {
          case expr::token::_not: return is_zero(x) ? true : false;
          case expr::token::neg: return -x;
          case expr::token::abs: return (x < 0) ? (-x) : x;
          case expr::token::_sin: return sin (x);
          case expr::token::_cos: return cos (x);
          case expr::token::_sqrt: return sqrt (x);
          case expr::token::_log: return log (x);
          case expr::token::_floor: return floor (x);
          case expr::token::_ceil: return ceil(x);
          case expr::token::_round:
            round_half_up = !round_half_up; 
            return round_half_up ? floor (x + 0.5) : ceil (x - 0.5);
          case expr::token::_tolong: return long(x);
          case expr::token::_todouble: return x;
          default: throw expr::exception::strange ("unary " + util::show(token));
          }
      }

      literal::type operator () (char & x) const
      {
        switch (token)
          {
          case expr::token::_len: return (long)(1);
          default:
            throw expr::exception::eval::type_error
              (util::show (token) + " ('" + util::show(x) + "')");
          }
      }

      literal::type operator () (std::string & x) const
      {
        switch (token)
          {
          case expr::token::_len: return (long)(x.size());
          default:
            throw expr::exception::eval::type_error
              (util::show (token) + " ('" + util::show(x) + "')");
          }
      }
    };

    class binary : public boost::static_visitor<literal::type>
    {
    private:
      const expr::token::type & token;
    public:
      binary (const expr::token::type & _token) : token (_token) {}

      literal::type operator () (control &, control &) const
      {
        throw expr::exception::eval::type_error 
          (util::show (token) + " for control token");
      }

      literal::type operator () (bool & l, bool & r) const
      {
        switch (token)
          {
          case expr::token::_or: return l || r;
          case expr::token::_and: return l && r;
          case expr::token::lt: return l < r ? true : false;
          case expr::token::le: return l <= r ? true : false;
          case expr::token::gt: return l > r ? true : false;
          case expr::token::ge: return l >= r ? true : false;
          case expr::token::ne: return l != r ? true : false;
          case expr::token::eq: return l == r ? true : false;
          case expr::token::add:
          case expr::token::sub:
          case expr::token::mul:
          case expr::token::div:
          case expr::token::divint:
          case expr::token::modint:
          case expr::token::mod:
          case expr::token::_pow:
          case expr::token::_powint:
          case expr::token::_bitset_insert:
          case expr::token::_bitset_delete:
          case expr::token::_bitset_is_element:
          case expr::token::_len:
          case expr::token::_substr:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type bool");
          case expr::token::min: return std::min (l,r);
          case expr::token::max: return std::max (l,r);
          default: throw expr::exception::strange ("binary " + util::show(token));
          }
      }

      literal::type operator () (long & l, long & r) const
      {
        switch (token)
          {
          case expr::token::_or: return l | r;
          case expr::token::_and: return l & r;
          case expr::token::lt: return l < r ? true : false;
          case expr::token::le: return l <= r ? true : false;
          case expr::token::gt: return l > r ? true : false;
          case expr::token::ge: return l >= r ? true : false;
          case expr::token::ne: return l != r ? true : false;
          case expr::token::eq: return l == r ? true : false;
          case expr::token::add: return l + r;
          case expr::token::sub: return l - r;
          case expr::token::mul: return l * r;
          case expr::token::div:
            if (r == 0) throw expr::exception::eval::divide_by_zero();
            return double(l) / double(r);
          case expr::token::divint:
            if (r == 0) throw expr::exception::eval::divide_by_zero();
            return l / r;
          case expr::token::modint:
          case expr::token::mod:
            if (r == 0) throw expr::exception::eval::divide_by_zero();
            return l % r;
          case expr::token::_pow: return pow (l, r);
          case expr::token::_powint:
            if (r < 0) throw expr::exception::eval::negative_exponent();
            {
              long x (1);
                
              for (long i (0); i < r; ++i) x *= l;

              return x;
            }
          case expr::token::min: return std::min (l,r);
          case expr::token::max: return std::max (l,r);
          case expr::token::_bitset_insert:
          case expr::token::_bitset_delete:
          case expr::token::_bitset_is_element:
          case expr::token::_len:
          case expr::token::_substr:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type long");
          default: throw expr::exception::strange ("binary " + util::show(token));
          }
      }

      literal::type operator () (double & l, double & r) const
      {
        switch (token)
          {
          case expr::token::_or:
          case expr::token::_and:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type double");
          case expr::token::lt: return is_zero (l-r) ? false : (l < r ? true : false);
          case expr::token::le: return is_zero (l-r) ? true : (l < r ? true : false);
          case expr::token::gt: return is_zero (l-r) ? false : (l > r ? true : false);
          case expr::token::ge: return is_zero (l-r) ? true : (l > r ? true : false);
          case expr::token::ne: return is_zero (l-r) ? false : true;
          case expr::token::eq: return is_zero (l-r) ? true : false;
          case expr::token::add: return l + r;
          case expr::token::sub: return l - r;
          case expr::token::mul: return l * r;
          case expr::token::div:
            if (is_zero (r)) throw expr::exception::eval::divide_by_zero();
            return double(l) / double(r);
          case expr::token::divint:
            if (is_zero (r)) throw expr::exception::eval::divide_by_zero();
            return l / r;
          case expr::token::modint:
          case expr::token::mod:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type double");
          case expr::token::_pow: return pow (l, r);
          case expr::token::_powint:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type double");
          case expr::token::min: return std::min (l,r);
          case expr::token::max: return std::max (l,r);
          case expr::token::_bitset_insert:
          case expr::token::_bitset_delete:
          case expr::token::_bitset_is_element:
          case expr::token::_len:
          case expr::token::_substr:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type double");
          default: throw expr::exception::strange ("binary " + util::show(token));
          }
      }

      literal::type operator () (std::string & l, std::string & r) const
      {
        switch (token)
          {
          case expr::token::_or:
          case expr::token::_and:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type string");
          case expr::token::lt: return l < r ? true : false;
          case expr::token::le: return l <= r ? true : false;
          case expr::token::gt: return l > r ? true : false;
          case expr::token::ge: return l >= r ? true : false;
          case expr::token::ne: return l != r ? true : false;
          case expr::token::eq: return l == r ? true : false;
          case expr::token::add: { std::string s; s+= l; s += r; return s; }
          case expr::token::sub:
          case expr::token::mul:
          case expr::token::div:
          case expr::token::divint:
          case expr::token::modint:
          case expr::token::mod:
          case expr::token::_pow:
          case expr::token::_powint:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type string");
          case expr::token::min: return std::min (l,r);
          case expr::token::max: return std::max (l,r);
          case expr::token::_bitset_insert:
          case expr::token::_bitset_delete:
          case expr::token::_bitset_is_element:
          case expr::token::_len:
          case expr::token::_substr:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type string");
          default: throw expr::exception::strange ("binary " + util::show(token));
          }
      }

      literal::type operator () (std::string & l, long & r) const
      {
        switch (token)
          {
          case expr::token::_substr:
            return l.substr(0, r);
          case expr::token::_or:
          case expr::token::_and:
          case expr::token::lt:
          case expr::token::le:
          case expr::token::gt:
          case expr::token::ge:
          case expr::token::ne:
          case expr::token::eq:
          case expr::token::add:
          case expr::token::sub:
          case expr::token::mul:
          case expr::token::div:
          case expr::token::divint:
          case expr::token::modint:
          case expr::token::mod:
          case expr::token::_pow:
          case expr::token::min:
          case expr::token::max:
          case expr::token::_powint:
          case expr::token::_bitset_insert:
          case expr::token::_bitset_delete:
          case expr::token::_bitset_is_element:
          case expr::token::_len:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type string and long");
          default: throw expr::exception::strange ("binary " + util::show(token));
          }
      }

      literal::type operator () (bitsetofint::type & set, long & l) const
      {
        switch (token)
          {
          case expr::token::_bitset_insert: set.ins (l); return set;
          case expr::token::_bitset_delete: set.del (l); return set;
          case expr::token::_bitset_is_element: return set.is_element (l);
          case expr::token::_substr:
          case expr::token::_or:
          case expr::token::_and:
          case expr::token::lt:
          case expr::token::le:
          case expr::token::gt:
          case expr::token::ge:
          case expr::token::ne:
          case expr::token::eq:
          case expr::token::add:
          case expr::token::sub:
          case expr::token::mul:
          case expr::token::div:
          case expr::token::divint:
          case expr::token::modint:
          case expr::token::mod:
          case expr::token::_pow:
          case expr::token::min:
          case expr::token::max:
          case expr::token::_powint:
          case expr::token::_len:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type bitset and long");
          default: throw expr::exception::strange ("binary " + util::show(token));
          }
      }

      literal::type operator () (char & l, char & r) const
      {
        switch (token)
          {
          case expr::token::_or:
          case expr::token::_and:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type char");
          case expr::token::lt: return l < r ? true : false;
          case expr::token::le: return l <= r ? true : false;
          case expr::token::gt: return l > r ? true : false;
          case expr::token::ge: return l >= r ? true : false;
          case expr::token::ne: return l != r ? true : false;
          case expr::token::eq: return l == r ? true : false;
          case expr::token::add: { std::string s; s+= l; s += r; return s; }
          case expr::token::sub:
          case expr::token::mul:
          case expr::token::div:
          case expr::token::divint:
          case expr::token::modint:
          case expr::token::mod:
          case expr::token::_pow:
          case expr::token::_powint:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type char");
          case expr::token::min: return std::min (l,r);
          case expr::token::max: return std::max (l,r);
          case expr::token::_bitset_insert:
          case expr::token::_bitset_delete:
          case expr::token::_bitset_is_element:
          case expr::token::_len:
          case expr::token::_substr:
            throw expr::exception::eval::type_error 
              (util::show (token) + " for values of type char");
          default: throw expr::exception::strange ("binary " + util::show(token));
          }
      }

      template<typename T,typename U>
      literal::type operator () (T & t, U & u) const
      {
        throw expr::exception::eval::type_error 
          ( util::show (token) 
          + " for values of wrong types: (" + show(t) + ", "  + show(u) + ")"
          );
      }
    };
  }
}

#endif
