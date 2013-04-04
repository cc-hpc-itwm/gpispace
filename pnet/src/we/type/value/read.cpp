// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/read.hpp>

#include <we/type/literal.hpp>
#include <we/type/literal/read.hpp>

#include <we/expr/exception.hpp>

namespace value
{
  type read (fhg::util::parse::position& pos)
  {
    pos.skip_spaces();

    if (not pos.end())
    {
      switch (*pos)
      {
      case '[': ++pos;
        if (pos.end())
        {
          throw expr::exception::parse::expected ("]", pos());
        }
        else if (*pos == ']')
        {
          ++pos;

          return we::type::literal::control();
        }
        else if (*pos == '.')
        {
          ++pos;

          if (pos.end() || *pos != ']')
          {
            throw expr::exception::parse::expected ("]", pos());
          }

          ++pos;

          return structured_t();
        }
        else
        {
          structured_t m;

          bool struct_closed (false);

          while (not pos.end() && not struct_closed)
          {
            const std::string name (pos.identifier());

            pos.skip_spaces();

            if (pos.end() or *pos != ':')
            {
              throw expr::exception::parse::expected (":", pos());
            }

            ++pos;

            if (pos.end() or *pos != '=')
            {
              throw expr::exception::parse::expected ("=", pos());
            }

            ++pos;

            m[name] = read (pos);

            pos.skip_spaces();

            if (pos.end())
            {
              throw expr::exception::parse::expected (", or ]", pos());
            }

            switch (*pos)
            {
            case ',': ++pos; break;
            case ']': ++pos; struct_closed = true; break;
            default:
              throw expr::exception::parse::expected (", or ]", pos());
            }
          }

          return m;
        }
        break;

      default:
        return literal::read (pos);
      }
    }

    return type();
  }

  type read (const std::string& s)
  {
    fhg::util::parse::position pos (s.begin(), s.end());

    return read (pos);
  }
}
