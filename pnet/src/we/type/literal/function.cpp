// mirko.rahn@itwm.fraunhofer.de

#include <we/type/literal/function.hpp>

#include <we/expr/token/type.hpp>
#include <we/expr/exception.hpp>

#include <we/type/literal.hpp>
#include <we/type/literal/show.hpp>
#include <we/type/bitsetofint.hpp>
#include <we/type/bytearray.hpp>

#include <fhg/util/show.hpp>

#include <boost/foreach.hpp>

#include <math.h>

namespace literal
{
  namespace function
  {
    namespace
    {
      class visitor_is_true : public boost::static_visitor<bool>
      {
      public:
        bool operator() (const bool& x) const
        {
          return x;
        }

        template<typename T>
        bool operator() (const T&) const
        {
          throw expr::exception::eval::type_error
            ("is_true for something that is not of type bool");
        }
      };

      bool is_zero (const double& x)
      {
        return (fabs (x) < 1e-6);
      }

      class visitor_unary : public boost::static_visitor<literal::type>
      {
      private:
        const expr::token::type& _token;

      public:
        visitor_unary (const expr::token::type& token)
          : _token (token)
        {}

        literal::type operator() (literal::map_type  m) const
        {
          switch (_token)
          {
          case expr::token::_map_size: return long (m.size());
          case expr::token::_map_empty: return m.empty();
          default:
            throw expr::exception::eval::type_error
              (fhg::util::show (_token) + " (" + literal::show (m) + ")");
          }
        }

        literal::type operator() (literal::set_type  s) const
        {
          switch (_token)
          {
          case expr::token::_set_pop: s.erase (s.begin()); return s;
          case expr::token::_set_top: return *(s.begin());
          case expr::token::_set_empty: return s.empty();
          case expr::token::_set_size: return long(s.size());
          default:
            throw expr::exception::eval::type_error
              (fhg::util::show (_token) + " (" + literal::show (s) + ")");
          }
        }

        literal::type operator() (literal::stack_type  s) const
        {
          switch (_token)
          {
          case expr::token::_stack_empty: return s.empty();
          case expr::token::_stack_top: return s.back();
          case expr::token::_stack_pop: s.pop_back(); return s;
          case expr::token::_stack_size: return long (s.size());
          default:
            throw expr::exception::eval::type_error
              (fhg::util::show (_token) + " (" + literal::show (s) + ")");
          }
        }

        literal::type operator() (bool  x) const
        {
          switch (_token)
          {
          case expr::token::_not: return !x;
          case expr::token::_tolong: return x ? 1L : 0L;
          case expr::token::_todouble: return x ? 1.0 : 0.0;
          default:
            throw expr::exception::eval::type_error
              (fhg::util::show (_token) + " (" + fhg::util::show(x) + ")");
          }
        }

        literal::type operator() (long  x) const
        {
          switch (_token)
          {
          case expr::token::_not: return x == 0;
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
          default:
            throw expr::exception::eval::type_error
              (fhg::util::show (_token) + " (" + fhg::util::show(x) + ")");
          }
        }

        literal::type operator() (double  x) const
        {
          static bool round_half_up (true);

          switch (_token)
          {
          case expr::token::_not: return is_zero(x);
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
          default:
            throw expr::exception::eval::type_error
              (fhg::util::show (_token) + " (" + fhg::util::show(x) + ")");
          }
        }

        literal::type operator() (char  x) const
        {
          switch (_token)
          {
          case expr::token::_len: return 1L;
          default:
            throw expr::exception::eval::type_error
              (fhg::util::show (_token) + " (" + fhg::util::show(x) + ")");
          }
        }

        literal::type operator() (std::string  x) const
        {
          switch (_token)
          {
          case expr::token::_len: return static_cast<long>(x.size());
          case expr::token::_bitset_fromhex: return bitsetofint::from_hex (x);
          default:
            throw expr::exception::eval::type_error
              (fhg::util::show (_token) + " (" + fhg::util::show(x) + ")");
          }
        }

        literal::type operator() (bitsetofint::type  b) const
        {
          switch (_token)
          {
          case expr::token::_bitset_tohex: return bitsetofint::to_hex (b);
          case expr::token::_bitset_count: return static_cast<long>(b.count());
          default:
            throw expr::exception::eval::type_error
              (fhg::util::show (_token) + " (" + fhg::util::show(b) + ")");
          }
        }

        template<typename T>
        literal::type operator() (T x) const
        {
          throw expr::exception::eval::type_error
            (fhg::util::show (_token) + " (" + literal::show (x) + ")");
        }
      };

      class visitor_binary : public boost::static_visitor<literal::type>
      {
      private:
        const expr::token::type& _token;

      public:
        visitor_binary (const expr::token::type& token)
          : _token (token)
        {}

        literal::type operator() ( literal::set_type  l
                                 , literal::set_type  r
                                 ) const
        {
          switch (_token)
          {
          case expr::token::_set_is_subset:
            BOOST_FOREACH (const long& lv, l)
            {
              if (!r.count (lv))
              {
                return false;
              }
            }
            return true;
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + literal::show (l) + "," + literal::show (r) + ")"
              );
          }
        }

        literal::type operator() (literal::set_type  s, long  k) const
        {
          switch (_token)
          {
          case expr::token::_set_insert: s.insert (k); return s;
          case expr::token::_set_erase: s.erase (k); return s;
          case expr::token::_set_is_element: return s.find (k) != s.end();
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + literal::show (s) + "," + fhg::util::show (k) + ")"
              );
          }
        }

        literal::type operator() (bool  l, bool  r) const
        {
          switch (_token)
          {
          case expr::token::_or: return l || r;
          case expr::token::_and: return l && r;
          case expr::token::lt: return l < r;
          case expr::token::le: return l <= r;
          case expr::token::gt: return l > r;
          case expr::token::ge: return l >= r;
          case expr::token::ne: return l != r;
          case expr::token::eq: return l == r;
          case expr::token::min: return std::min (l,r);
          case expr::token::max: return std::max (l,r);
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + fhg::util::show (l) + "," + fhg::util::show (r) + ")"
              );
          }
        }

        literal::type operator() (long  l, long  r) const
        {
          switch (_token)
          {
          case expr::token::_or: return l | r;
          case expr::token::_and: return l & r;
          case expr::token::lt: return l < r;
          case expr::token::le: return l <= r;
          case expr::token::gt: return l > r;
          case expr::token::ge: return l >= r;
          case expr::token::ne: return l != r;
          case expr::token::eq: return l == r;
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
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + fhg::util::show (l) + "," + fhg::util::show (r) + ")"
              );
          }
        }

        literal::type operator() (double  l, double  r) const
        {
          switch (_token)
          {
          case expr::token::lt: return is_zero (l-r) ? false : (l < r);
          case expr::token::le: return is_zero (l-r) ? true : (l < r);
          case expr::token::gt: return is_zero (l-r) ? false : (l > r);
          case expr::token::ge: return is_zero (l-r) ? true : (l > r);
          case expr::token::ne: return !is_zero (l-r);
          case expr::token::eq: return is_zero (l-r);
          case expr::token::add: return l + r;
          case expr::token::sub: return l - r;
          case expr::token::mul: return l * r;
          case expr::token::div:
            if (is_zero (r)) throw expr::exception::eval::divide_by_zero();
            return l / r;
          case expr::token::divint:
            if (is_zero (r)) throw expr::exception::eval::divide_by_zero();
            return long (l / r);
          case expr::token::_pow: return pow (l, r);
          case expr::token::min: return std::min (l,r);
          case expr::token::max: return std::max (l,r);
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + fhg::util::show (l) + "," + fhg::util::show (r) + ")"
              );
          }
        }

        literal::type operator() (std::string  l, std::string  r) const
        {
          switch (_token)
          {
          case expr::token::lt: return l < r;
          case expr::token::le: return l <= r;
          case expr::token::gt: return l > r;
          case expr::token::ge: return l >= r;
          case expr::token::ne: return l != r;
          case expr::token::eq: return l == r;
          case expr::token::add: { std::string s; s+= l; s += r; return s; }
          case expr::token::min: return std::min (l,r);
          case expr::token::max: return std::max (l,r);
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + fhg::util::show (l) + "," + fhg::util::show (r) + ")"
              );
          }
        }

        literal::type operator() (std::string  l, long  r) const
        {
          switch (_token)
          {
          case expr::token::_substr: return l.substr(0, r);
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + fhg::util::show (l) + "," + fhg::util::show (r) + ")"
              );
          }
        }

        literal::type operator() (literal::stack_type  s, long  l) const
        {
          switch (_token)
          {
          case expr::token::_stack_push: s.push_back (l); return s;
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + literal::show (s) + "," + fhg::util::show (l) + ")"
              );
          }
        }

        literal::type operator() ( literal::stack_type  r
                                 , literal::stack_type  l
                                 ) const
        {
          switch (_token)
          {
          case expr::token::_stack_join:
            while (!r.empty())
            {
              l.push_back (r.front()); r.pop_front();
            }
            return l;
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + literal::show (l) + "," + literal::show (r) + ")"
              );
          }
        }

        literal::type operator() (bitsetofint::type  set, long  l) const
        {
          switch (_token)
          {
          case expr::token::_bitset_insert: set.ins (l); return set;
          case expr::token::_bitset_delete: set.del (l); return set;
          case expr::token::_bitset_is_element: return set.is_element (l);
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + fhg::util::show (set) + "," + fhg::util::show (l) + ")"
              );
          }
        }

        literal::type operator() (literal::map_type  m, long  k) const
        {
          switch (_token)
          {
          case expr::token::_map_unassign: m.erase (k); return m;
          case expr::token::_map_is_assigned: return m.find (k) != m.end();
          case expr::token::_map_get_assignment: return m.at (k);
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + literal::show (m) + "," + fhg::util::show (k) + ")"
              );
          }
        }

        literal::type operator() (char  l, char  r) const
        {
          switch (_token)
          {
          case expr::token::lt: return l < r;
          case expr::token::le: return l <= r;
          case expr::token::gt: return l > r;
          case expr::token::ge: return l >= r;
          case expr::token::ne: return l != r;
          case expr::token::eq: return l == r;
          case expr::token::add: { std::string s; s+= l; s += r; return s; }
          case expr::token::min: return std::min (l,r);
          case expr::token::max: return std::max (l,r);
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + fhg::util::show (l) + "," + fhg::util::show (r) + ")"
              );
          }
        }

        literal::type operator() ( bitsetofint::type  l
                                 , bitsetofint::type  r
                                 ) const
        {
          switch (_token)
          {
          case expr::token::eq: return l == r;
          case expr::token::_bitset_or: return l | r;
          case expr::token::_bitset_and: return l & r;
          case expr::token::_bitset_xor: return l ^ r;
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + fhg::util::show (l) + "," + fhg::util::show (r) + ")"
              );
          }
        }

        literal::type operator() ( bytearray::type  l
                                 , bytearray::type  r
                                 ) const
        {
          switch (_token)
          {
          case expr::token::eq: return l == r;
          default:
            throw expr::exception::eval::type_error
              ( fhg::util::show (_token) +
              "(" + fhg::util::show (l) + "," + fhg::util::show (r) + ")"
              );
          }
        }

        template<typename T,typename U>
        literal::type operator() (T  t, U  u) const
        {
          throw expr::exception::eval::type_error
            ( fhg::util::show (_token) +
            "(" + literal::show (t) + "," + literal::show (u) + ")"
            );
        }
      };
    }

    bool is_true (const literal::type& v)
    {
      return boost::apply_visitor (visitor_is_true(), v);
    }

    literal::type unary (const expr::token::type& token, literal::type  x)
    {
      return boost::apply_visitor (visitor_unary (token), x);
    }

    literal::type binary ( const expr::token::type& token
                         , literal::type  x
                         , literal::type  y
                         )
    {
      return boost::apply_visitor (visitor_binary (token), x, y);
    }
  }
}
