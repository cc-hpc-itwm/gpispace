// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/read.hpp>

#include <fhg/util/parse/error.hpp>

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
        void struct_item ( std::map<std::string, value_type>& m
                         , fhg::util::parse::position& pos
                         )
        {
          const std::string fieldname (pos.identifier());

          pos.skip_spaces();
          pos.require (":=");

          m[fieldname] = read (pos);
        }
        void vector_item ( std::vector<value_type>& v
                         , fhg::util::parse::position& pos
                         )
        {
          v.push_back (read (pos));
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
            const char c (pos.character());
            pos.require ("'");
            return c;
          }

        case '"':
          ++pos;
          return pos.until ('\"');

        case 'm':
          {
            ++pos;
            pos.require ("ap");

            std::map<value_type, value_type> m;

            pos.list ( '[', ',', ']'
                     , boost::bind (map_item, boost::ref (m), _1)
                     );

            return m;
          }

        case 's':
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

              std::map<std::string, value_type> m;

              pos.list ( '[', ',', ']'
                       , boost::bind (struct_item, boost::ref (m), _1)
                       );

              return m;
            }
          default:
            throw fhg::util::parse::error::expected ("truct, et", pos);
          }
          break;

        case 'v':
          {
            ++pos;
            pos.require ("ector");

            std::vector<value_type> v;

            pos.list ( '(', ',', ')'
                     , boost::bind (vector_item, boost::ref (v), _1)
                     );

            return v;
          }

        case 'l':
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
          throw std::runtime_error ("not yet implemented: " + pos.rest());
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
