// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/read.hpp>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/require.hpp>
#include <fhg/util/num.hpp>

#include <boost/bind.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        void map_item ( std::map<value_type, value_type>& m
                      , fhg::util::parse::position& pos
                      )
        {
          const value_type key (read (pos));

          pos.skip_spaces();
          pos.require ("->");

          m[key] = read (pos);
        }
        void struct_item ( structured_type& m
                         , fhg::util::parse::position& pos
                         )
        {
          pos.skip_spaces();
          const std::string fieldname
            (fhg::util::parse::require::identifier (pos));

          pos.skip_spaces();
          pos.require (":=");

          m.push_back (std::make_pair (fieldname, read (pos)));
        }
        void list_item ( std::list<value_type>& l
                       , fhg::util::parse::position& pos
                       )
        {
          l.push_back (read (pos));
        }
        void set_item ( std::set<value_type>& s
                      , fhg::util::parse::position& pos
                      )
        {
          s.insert (read (pos));
        }
      }

      value_type read (fhg::util::parse::position& pos)
      {
        pos.skip_spaces();

        if (pos.end())
        {
          throw fhg::util::parse::error::expected ("value", pos);
        }

        switch (*pos)
        {
        case '[':
          ++pos;
          pos.require ("]");
          return we::type::literal::control();

        case 't':
          ++pos;
          pos.require ("rue");
          return true;

        case 'f':
          ++pos;
          pos.require ("alse");
          return false;

        case '\'':
          {
            ++pos;
            const char c (fhg::util::parse::require::plain_character (pos));
            pos.require ("'");
            return c;
          }

        case '"':
          ++pos;
          return pos.until ('\"');

        case '{':
          {
            ++pos;

            bitsetofint::type bs;

            pos.skip_spaces();

            while (!pos.end() && *pos != '}')
            {
              bs.push_back (fhg::util::read_ulong (pos));

              pos.skip_spaces();
            }

            pos.require ("}");

            return bs;
          }

        case 'y':
          {
            ++pos;

            pos.require ("(");

            bytearray::type ba;

            pos.skip_spaces();

            while (!pos.end() && *pos != ')')
            {
              ba.push_back (fhg::util::read_ulong (pos));

              pos.skip_spaces();
            }

            pos.require (")");

            return ba;
          }

        case 'M':
          {
            ++pos;
            pos.require ("ap");

            std::map<value_type, value_type> m;

            pos.list ( '[', ',', ']'
                     , boost::bind (map_item, boost::ref (m), _1)
                     );

            return m;
          }

        case 'S':
          ++pos;
          if (pos.end())
          {
            throw fhg::util::parse::error::expected ("truct, et", pos);
          }
          else switch (*pos)
          {
          case 'e':
            {
              ++pos;
              pos.require ("t");

              std::set<value_type> s;

              pos.list ( '{', ',', '}'
                       , boost::bind (set_item, boost::ref (s), _1)
                       );

              return s;
            }
          case 't':
            {
              ++pos;
              pos.require ("ruct");

              structured_type m;

              pos.list ( '[', ',', ']'
                       , boost::bind (struct_item, boost::ref (m), _1)
                       );

              return m;
            }
          default:
            throw fhg::util::parse::error::expected ("truct, et", pos);
          }
          break;

        case 'L':
          {
            ++pos;
            pos.require ("ist");

            std::list<value_type> l;

            pos.list ( '(', ',', ')'
                     , boost::bind (list_item, boost::ref (l), _1)
                     );

            return l;
          }

        default:
          return fhg::util::read_num (pos);
        }
      }

      value_type read (const std::string& input)
      {
        fhg::util::parse::position pos (input.begin(), input.end());

        return read (pos);
      }
    }
  }
}
