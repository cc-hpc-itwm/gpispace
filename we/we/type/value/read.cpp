// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/read.hpp>

#include <we/type/literal.hpp>
#include <we/type/literal/read.hpp>

#include <we/expr/exception.hpp>

namespace value
{
  namespace
  {
    void skip_spaces (fhg::util::parse::position& pos)
    {
      while (!pos.end() && isspace (*pos))
      {
        ++pos;
      }
    }
  }

  type read (fhg::util::parse::position& pos)
  {
    type v;

    skip_spaces (pos);

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
          v = structured_t();
          structured_t& m (boost::get<structured_t&> (v));

          bool struct_closed (false);

          while (not pos.end() && not struct_closed)
          {
            const std::string name (identifier (pos));

            if (name.empty())
            {
              throw expr::exception::parse::expected ("identifier", pos());
            }

            skip_spaces (pos);

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

            skip_spaces (pos);

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
        }
        break;

      default:
        {
          v = literal::type();

          literal::read (boost::get<literal::type&>(v), pos);
        }
      }
    }

    return v;
  }

  std::string identifier (fhg::util::parse::position& pos)
  {
    skip_spaces (pos);

    return literal::identifier (pos);
  }

  type read (const std::string& s)
  {
    std::size_t k (0);
    std::string::const_iterator begin (s.begin());

    fhg::util::parse::position pos (k, begin, s.end());

    return read (pos);
  }
}
