// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_HPP
#define _WE_TYPE_LITERAL_HPP

#include <we/expr/parse/position.hpp>

#include <we/expr/exception.hpp>

#include <we/util/show.hpp>

#include <we/type/signature.hpp>
#include <we/type/control.hpp>
#include <we/type/bitsetofint.hpp>

#include <we/type/literal/name.hpp>

#include <boost/variant.hpp>

#include <boost/functional/hash.hpp>

#include <string>
#include <stdexcept>

#include <iostream>

namespace literal
{
  typedef boost::variant< control
                        , bool
                        , long
                        , double
                        , char
                        , std::string
                        , bitsetofint::type
                        > type;

  class visitor_type_name : public boost::static_visitor<type_name_t>
  {
  public:
    type_name_t operator () (const control &) const { return CONTROL; }
    type_name_t operator () (const bool &) const { return BOOL; }
    type_name_t operator () (const long &) const { return LONG; }
    type_name_t operator () (const double &) const { return DOUBLE; }
    type_name_t operator () (const char &) const { return CHAR; }
    type_name_t operator () (const std::string &) const { return STRING; }
    type_name_t operator () (const bitsetofint::type &) const { return BITSET; }
  };

  class visitor_show : public boost::static_visitor<std::string>
  {
  public:
    std::string operator () (const bool & x) const
    {
      return x ? "true" : "false";
    }

    std::string operator () (const long & x) const
    {
      return util::show (x) + "L";
    }

    std::string operator () (const char & x) const
    {
      return "'" + util::show (x) + "'";
    }

    std::string operator () (const std::string & x) const
    {
      return "\"" + x + "\"";     
    }

    template<typename T>
    std::string operator () (const T & x) const
    {
      return util::show (x);
    }
  };

  static std::string show (const type v)
  {
    return boost::apply_visitor (visitor_show(), v);
  }

  class visitor_hash : public boost::static_visitor<std::size_t>
  {
  public:
    std::size_t operator () (const control &) const
    {
      return 42;
    }

    template<typename T>
    std::size_t operator () (const T & x) const
    {
      boost::hash<T> hasher;

      return hasher(x);
    }
  };

  static char read_quoted_char (expr::parse::position & pos)
  {
    const unsigned int open (pos());

    if (*pos == '\\')
      ++pos;
      
    if (pos.end())
      throw expr::exception::parse::unterminated ("\\", open, pos());

    const char c (*pos);

    ++pos;

    return c;
  }

  static long read_ulong (expr::parse::position & pos)
  {
    if (pos.end() || !isdigit(*pos))
      throw expr::exception::parse::expected ("digit", pos());

    long l (0);
      
    while (!pos.end() && isdigit(*pos))
      {
        l *= 10;
        l += *pos - '0';
        ++pos;
      }

    return l;
  }

  static bool read_sign (expr::parse::position & pos)
  {
    if (!pos.end() && *pos == '-')
      {
        ++pos;
        return true;
      }

    return false;
  }

  static long read_long (expr::parse::position & pos)
  {
    const bool negative (read_sign(pos));

    return negative ? -read_ulong (pos) : read_ulong (pos);
  }

  static double read_fraction (const long ipart, expr::parse::position & pos)
  {
    double d (ipart);
    double frac (read_ulong (pos));

    while (frac > 1)
      frac /= 10;

    d = (d < 0) ? (d - frac) : (d + frac);

    if (!pos.end() && *pos == 'e')
      {
        ++pos;
        const bool negative (read_sign (pos));
        long ex (read_ulong (pos));

        if (negative)
          d /= pow (10, ex);
        else
          d *= pow (10, ex);
      }

    return d;
  }

  static void read_num (type & v, expr::parse::position & pos)
  {
    long i (read_long (pos));

    if (pos.end() || *pos != '.')
      {
        v = i;

        // consume a trailing 'L'
        if (!pos.end() && *pos == 'L')
          ++pos;
      }
    else
      {
        ++pos;
        v = read_fraction (i, pos);
      }
  }

  static void read (type & v, expr::parse::position & pos)
  {
    if (pos.end())
      throw expr::exception::parse::expected
        ("long or double or char or string", pos());

    switch (*pos)
      {
      case '\'':
        {
          const unsigned int open (pos());

          ++pos;

          if (pos.end())
            throw expr::exception::parse::missing ("character after '", pos());

          if (*pos == '\'')
            throw expr::exception::parse::exception ("'' with no content", pos());

          v = read_quoted_char (pos);

          if (pos.end() || *pos != '\'')
            throw expr::exception::parse::unterminated ("'", open, pos());

          ++pos;
        }
        break;
      case '"':
        {
          const unsigned int open (pos());

          ++pos;

          std::string s;

          while (!pos.end() && *pos != '"')
            s.push_back (read_quoted_char (pos));

          if (pos.end())
            throw expr::exception::parse::unterminated ("\"", open, pos());

          ++pos;

          v = s;
        }
        break;
      case '{':
        {
          const unsigned int open (pos());

          ++pos;

          bitsetofint::type::container_type container;

          do
            {
              if (pos.end())
                throw expr::exception::parse::unterminated ("{", open, pos());
              else
                switch (*pos)
                  {
                  case '}': break;
                  default:
                    container.push_back (read_ulong (pos));
                    if (pos.end())
                      throw expr::exception::parse::unterminated 
                        ("{", open, pos());
                    else
                      switch (*pos)
                        {
                        case '}': break;
                        case ',': ++pos; break;
                        default:
                          throw expr::exception::parse::expected 
                            ("'}' or ','", pos());
                        }
                    break;
                  }
            }
          while (!(pos.end() || *pos == '}'));

          if (pos.end())
            throw expr::exception::parse::unterminated ("{", open, pos());

          ++pos;

          v = bitsetofint::type (container);
        }
        break;
      default:
        read_num (v, pos);
      }
  }

  namespace exception
  {
    class type_error : public std::runtime_error
    {
    public:
      type_error (const std::string & what)
        : std::runtime_error ("type error: " + what) {};

      type_error ( const std::string & field
                 , const std::string & required
                 , const std::string & given
                 )
        : std::runtime_error ( "type error: " + field 
                             + " requires value of type " + required 
                             + ", given value of type " + given
                             ) {};
    };
  }

  inline const type & require_type ( const signature::field_name_t & field
                                   , const type_name_t & req
                                   , const type & x
                                   )
  {
    const type_name_t has (boost::apply_visitor (visitor_type_name(), x));

    if (has != req)
      throw exception::type_error (field, req, has);

    return x;
  }
}

namespace boost
{
  static inline std::size_t hash_value (const literal::type & v)
  {
    return boost::apply_visitor (literal::visitor_hash(), v);
  }
}

#endif
