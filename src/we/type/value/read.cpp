// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/read.hpp>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>
#include <fhg/util/num.hpp>

#include <functional>

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

          fhg::util::parse::require::skip_spaces (pos);
          fhg::util::parse::require::require (pos, "->");

          m[key] = read (pos);
        }
        void struct_item ( structured_type& m
                         , fhg::util::parse::position& pos
                         )
        {
          fhg::util::parse::require::skip_spaces (pos);
          const std::string fieldname
            (fhg::util::parse::require::identifier (pos));

          fhg::util::parse::require::skip_spaces (pos);
          fhg::util::parse::require::require (pos, ":=");

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
        fhg::util::parse::require::skip_spaces (pos);

        if (pos.end())
        {
          throw fhg::util::parse::error::expected ("value", pos);
        }

        switch (*pos)
        {
        case '[':
          ++pos;
          fhg::util::parse::require::require (pos, ']');
          return we::type::literal::control();

        case 't':
          ++pos;
          fhg::util::parse::require::require (pos, "rue");
          return true;

        case 'f':
          ++pos;
          fhg::util::parse::require::require (pos, "alse");
          return false;

        case '\'':
          {
            ++pos;
            const char c (fhg::util::parse::require::plain_character (pos));
            fhg::util::parse::require::require (pos, '\'');
            return c;
          }

        case '"':
          ++pos;
          return fhg::util::parse::require::plain_string (pos, '\"');

        case '{':
          {
            ++pos;

            bitsetofint::type bs;

            fhg::util::parse::require::skip_spaces (pos);

            while (!pos.end() && *pos != '}')
            {
              bs.push_back (fhg::util::read_ulong (pos));

              fhg::util::parse::require::skip_spaces (pos);
            }

            fhg::util::parse::require::require (pos, '}');

            return bs;
          }

        case 'y':
          {
            ++pos;

            fhg::util::parse::require::require (pos, '(');

            bytearray::type ba;

            fhg::util::parse::require::skip_spaces (pos);

            while (!pos.end() && *pos != ')')
            {
              ba.push_back (fhg::util::read_ulong (pos));

              fhg::util::parse::require::skip_spaces (pos);
            }

            fhg::util::parse::require::require (pos, ')');

            return ba;
          }

        case 'M':
          {
            ++pos;
            fhg::util::parse::require::require (pos, "ap");

            std::map<value_type, value_type> m;

            fhg::util::parse::require::list
              ( pos
              , '[', ',', ']'
              , std::bind (map_item, std::ref (m), std::placeholders::_1)
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
              fhg::util::parse::require::require (pos, 't');

              std::set<value_type> s;

              fhg::util::parse::require::list
                ( pos
                ,  '{', ',', '}'
                , std::bind (set_item, std::ref (s), std::placeholders::_1)
                );

              return s;
            }
          case 't':
            {
              ++pos;
              fhg::util::parse::require::require (pos, "ruct");

              structured_type m;

              fhg::util::parse::require::list
                ( pos
                ,  '[', ',', ']'
                , std::bind (struct_item, std::ref (m), std::placeholders::_1)
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
            fhg::util::parse::require::require (pos, "ist");

            std::list<value_type> l;

            fhg::util::parse::require::list
              ( pos
              ,  '(', ',', ')'
              , std::bind (list_item, std::ref (l), std::placeholders::_1)
              );

            return l;
          }

        default:
          return fhg::util::read_num (pos);
        }
      }

      value_type read (const std::string& input)
      {
        fhg::util::parse::position_string pos (input);

        return read (pos);
      }
    }
  }
}
