// mirko.rahn@itwm.fraunhofer.de

#include <we/type/literal/read.hpp>

#include <we/expr/exception.hpp>

#include <fhg/util/num.hpp>

namespace literal
{
  namespace
  {
    class visitor_project : public boost::static_visitor<type>
    {
    public:
      type operator() (const unsigned int& x) const
      {
        return static_cast<long>(x);
      }
      type operator() (const int& x) const
      {
        return static_cast<long>(x);
      }
      type operator() (const unsigned long& x) const
      {
        return static_cast<long>(x);
      }
      type operator() (const long& x) const
      {
        return x;
      }
      type operator() (const float& x) const
      {
        return static_cast<double>(x);
      }
      type operator() (const double& x) const
      {
        return x;
      }
    };

    type project (const fhg::util::num_type& n)
    {
      return boost::apply_visitor (visitor_project(), n);
    }
  }

  namespace
  {
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

      l = fhg::util::read_long (pos);

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

      key = fhg::util::read_long (pos); pos.skip_spaces();

      pos.require ("->"); pos.skip_spaces();

      val = fhg::util::read_long (pos); pos.skip_spaces();

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
        ++pos;
        const char c (pos.character());
        pos.require ("'");
        return c;
      }
    case '"':
      {
        ++pos;
        return pos.until ('"');
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
      return project (fhg::util::read_num (pos));
    }
  }
}
