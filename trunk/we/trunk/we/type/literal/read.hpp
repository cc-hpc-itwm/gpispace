// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_LITERAL_READ_HPP
#define _WE_TYPE_LITERAL_READ_HPP

#include <fhg/util/parse/position.hpp>

#include <we/expr/exception.hpp>

#include <we/type/literal.hpp>


namespace literal
{
  static char read_quoted_char (fhg::util::parse::position & pos)
  {
    const std::size_t open (pos());

    if (*pos == '\\')
      ++pos;

    if (pos.end())
      throw expr::exception::parse::unterminated ("\\", open, pos());

    const char c (*pos);

    ++pos;

    return c;
  }

  struct hex
  {
    static inline bool isdig (const char & c)
    {
      switch (c)
        {
        case '0'...'9':
        case 'a'...'e':
        case 'A'...'E': return true;
        default: return false;
        }
    }

    static inline long val (const char & c)
    {
      switch (c)
        {
        case '0'...'9': return       c - '0';
        case 'a'...'e': return 10 + (c - 'a');
        case 'A'...'E': return 10 + (c - 'A');
        default:
          throw std::runtime_error ("hexval for non hexdigit");
        }
    }

    static const long base = 16;
  };

  struct dec
  {
    static inline bool isdig (const char & c)
    {
      return isdigit (c);
    }

    static inline long val (const char & c)
    {
      return c - '0';
    }

    static const long base = 10;
  };

  template<typename base>
  static inline long generic_read_ulong (fhg::util::parse::position & pos)
  {
    long l (0);

    while (!pos.end() && base::isdig (*pos))
      {
        l *= base::base;
        l += base::val (*pos);
        ++pos;
      }

    return l;
  }

  static long read_ulong (fhg::util::parse::position & pos)
  {
    if (pos.end() || !isdigit(*pos))
      {
        throw expr::exception::parse::expected ("digit", pos());
      }

    long l (0);
    bool zero (false);

    while (!pos.end() && *pos == '0')
      {
        ++pos; zero = true;
      }

    if (!pos.end())
      {
        if (zero && (*pos == 'x' || *pos == 'X'))
          {
            ++pos;
            l = generic_read_ulong<hex>(pos);
          }
        else
          {
            l = generic_read_ulong<dec>(pos);
          }
      }

    return l;
  }

  static bool read_sign (fhg::util::parse::position & pos)
  {
    if (!pos.end() && *pos == '-')
      {
        ++pos;
        return true;
      }

    return false;
  }

  static long read_long (fhg::util::parse::position & pos)
  {
    const bool negative (read_sign(pos));

    return negative ? -read_ulong (pos) : read_ulong (pos);
  }

  static double read_fraction ( const long ipart
                              , fhg::util::parse::position & pos
                              )
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

  static void read_num (type & v, fhg::util::parse::position & pos)
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

  inline void require ( const std::string & what
                      , fhg::util::parse::position & pos
                      )
  {
    std::string::const_iterator what_pos (what.begin());
    const std::string::const_iterator what_end (what.end());

    while (what_pos != what_end)
      if (pos.end() || *pos != *what_pos)
        throw expr::exception::parse::expected
          ("'" + std::string (what_pos, what_end) + "'", pos());
      else
        {
          ++pos; ++what_pos;
        }
  }

  static void read (type & v, fhg::util::parse::position & pos)
  {
    if (pos.end())
      throw expr::exception::parse::expected
        ("long or double or char or string", pos());

    switch (*pos)
      {
      case 't': ++pos; require ("rue", pos); v = true; break;
      case 'f': ++pos; require ("alse", pos); v = false; break;
      case '[': ++pos; require ("]", pos); v = control(); break;
      case '@': ++pos; require ("@", pos); v = literal::stack_type(); break;
      case '\'':
        {
          const std::size_t open (pos());

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
          const std::size_t open (pos());

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
          const std::size_t open (pos());

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
}

#endif
