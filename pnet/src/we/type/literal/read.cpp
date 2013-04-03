// mirko.rahn@itwm.fraunhofer.de

#include <we/type/literal/read.hpp>

#include <we/expr/exception.hpp>

namespace literal
{
  std::string identifier (fhg::util::parse::position& pos)
  {
    std::string x;

    if (pos.end() or not (isalpha (*pos) or *pos == '_'))
    {
      throw expr::exception::parse::expected ("identifier", pos());
    }

    x.push_back (*pos); ++pos;

    while (not pos.end() and (isalpha (*pos) or isdigit (*pos) or *pos == '_'))
    {
      x.push_back (*pos); ++pos;
    }

    return x;
  }

  namespace
  {
    char read_quoted_char (fhg::util::parse::position & pos)
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
        case 'a'...'f':
        case 'A'...'F': return true;
        default: return isdigit (c);
        }
      }

      static inline long val (const char & c)
      {
        switch (c)
        {
        case 'a'...'f': return 10 + (c - 'a');
        case 'A'...'F': return 10 + (c - 'A');
        default: return c - '0';
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
    inline long generic_read_ulong (fhg::util::parse::position & pos)
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

    long read_ulong (fhg::util::parse::position & pos)
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

    bool read_sign (fhg::util::parse::position & pos)
    {
      if (!pos.end() && *pos == '-')
      {
        ++pos;
        return true;
      }

      return false;
    }

    long read_long (fhg::util::parse::position & pos)
    {
      const bool negative (read_sign(pos));

      return negative ? -read_ulong (pos) : read_ulong (pos);
    }

    double read_fraction ( const long ipart
                         , fhg::util::parse::position & pos
                         )
    {
      double d (ipart);

      double frac (0.0);
      double fak (0.1);

      while (!pos.end() && isdigit (*pos))
      {
        frac += double (*pos - '0') * fak;
        fak *= 0.1;
        ++pos;
      }

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

    type read_num (fhg::util::parse::position & pos)
    {
      long i (read_long (pos));

      if (pos.end() || *pos != '.')
      {
        // consume a trailing 'L'
        if (!pos.end() && *pos == 'L')
        {
          ++pos;
        }

        return i;
      }
      else
      {
        ++pos;
        return read_fraction (i, pos);
      }
    }

    void skip_sep (const char & sep, fhg::util::parse::position & pos)
    {
      if (!pos.end())
      {
        if (*pos == sep)
        {
          ++pos; pos.skip_spaces();
        }
      }
    }

    bool read_list_item (long & l, fhg::util::parse::position & pos)
    {
      if (pos.end() || !isspace (*pos))
      {
        return false;
      }

      ++pos;

      pos.skip_spaces();

      l = read_long (pos);

      return true;
    }

    bool read_map_item ( long & key
                       , long & val
                       , fhg::util::parse::position & pos
                       )
    {
      if (pos.end() || !isdigit (*pos))
      {
        return false;
      }

      key = read_long (pos); pos.skip_spaces();

      pos.require ("->"); pos.skip_spaces();

      val = read_long (pos); pos.skip_spaces();

      skip_sep (',', pos);

      return true;
    }
  }

  type read (fhg::util::parse::position & pos)
  {
    if (pos.end())
    {
      throw expr::exception::parse::expected
        ("long or double or char or string", pos());
    }

    switch (*pos)
    {
    case 't': ++pos; pos.require ("rue"); return true;
    case 'f': ++pos; pos.require ("alse"); return false;
    case 'y': ++pos;
      {
        pos.require ("(");

        bytearray::type ba;
        long l;

        while (read_list_item (l, pos))
        {
          ba.push_back (l);
        }

        pos.require (")");

        return ba;
      }
    case '[': ++pos; pos.require ("]"); return we::type::literal::control();
    case '@': ++pos;
      {
        literal::stack_type s;
        long l;

        while (read_list_item (l, pos))
        {
          s.push_front (l);
        }

        pos.require ("@");

        return s;
      }
    case '\'':
      {
        const std::size_t open (pos());

        ++pos;

        if (pos.end())
          throw expr::exception::parse::missing ("character after '", pos());

        if (*pos == '\'')
          throw expr::exception::parse::exception ("'' with no content", pos());

        char c (read_quoted_char (pos));

        if (pos.end() || *pos != '\'')
          throw expr::exception::parse::unterminated ("'", open, pos());

        ++pos;

        return c;
      }
    case '"':
      {
        const std::size_t open (pos());

        ++pos;

        std::string s;

        while (!pos.end() && *pos != '"')
        {
          s.push_back (read_quoted_char (pos));
        }

        if (pos.end())
        {
          throw expr::exception::parse::unterminated ("\"", open, pos());
        }

        ++pos;

        return s;
      }
    case '{': ++pos;
      if (pos.end())
        throw expr::exception::parse::expected ("'}' or ':' or '|'", pos());
      else
        switch (*pos)
        {
        case ':': ++pos;
          {
            literal::set_type s;
            long l;

            while (read_list_item (l, pos))
            {
              s.insert (l);
            }

            pos.require (":}");

            return s;
          }
        case '|': ++pos;
          {
            literal::map_type m;
            long key;
            long val;

            while (read_map_item (key, val, pos))
            {
              m.insert (std::make_pair (key, val));
            }

            pos.require ("|}");

            return m;
          }
        default:
          {
            bitsetofint::type bs;
            long l;

            while (read_list_item (l, pos))
            {
              bs.push_back (l);
            }

            pos.require ("}");

            return bs;
          }
        }
    default:
      return read_num (pos);
    }
  }
}
