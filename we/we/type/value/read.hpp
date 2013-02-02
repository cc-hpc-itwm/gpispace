// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_READ_HPP
#define _WE_TYPE_VALUE_READ_HPP 1

#include <fhg/util/parse/position.hpp>

#include <we/type/value.hpp>
#include <we/type/literal.hpp>
#include <we/type/literal/read.hpp>

#include <we/expr/exception.hpp>

namespace value
{
  namespace detail
  {
    inline void skip_spaces (fhg::util::parse::position& pos)
    {
      while (!pos.end() && isspace (*pos))
        {
          ++pos;
        }
    }
  }

  inline std::string identifier (fhg::util::parse::position& pos)
  {
    detail::skip_spaces (pos);

    return literal::identifier (pos);
  }

  inline type read (fhg::util::parse::position& pos)
  {
    type v;

    detail::skip_spaces (pos);

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

                    detail::skip_spaces (pos);

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

                    detail::skip_spaces (pos);

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

  inline type read (const std::string& s)
  {
    std::size_t k (0);
    std::string::const_iterator begin (s.begin());

    fhg::util::parse::position pos (k, begin, s.end());

    return read (pos);
  }
}

#endif
