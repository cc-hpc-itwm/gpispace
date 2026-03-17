// Copyright (C) 2013-2015,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/value/read.hpp>

#include <gspc/util/num.hpp>
#include <gspc/util/parse/error.hpp>
#include <gspc/util/parse/position.hpp>
#include <gspc/util/parse/require.hpp>
#include <gspc/util/functor_visitor.hpp>
#include <gspc/we/type/shared.hpp>

#include <cctype>
#include <functional>



    namespace gspc::pnet::type::value
    {
      namespace
      {
        void map_item ( std::map<value_type, value_type>& m
                      , util::parse::position& pos
                      )
        {
          const value_type key (read (pos));

          util::parse::require::skip_spaces (pos);
          util::parse::require::require (pos, "->");

          m[key] = read (pos);
        }
        void struct_item ( structured_type& m
                         , util::parse::position& pos
                         )
        {
          util::parse::require::skip_spaces (pos);
          const std::string fieldname
            (util::parse::require::identifier (pos));

          util::parse::require::skip_spaces (pos);
          util::parse::require::require (pos, ":=");

          m.emplace_back (fieldname, read (pos));
        }
        void list_item ( std::list<value_type>& l
                       , util::parse::position& pos
                       )
        {
          l.emplace_back (read (pos));
        }
        void set_item ( std::set<value_type>& s
                      , util::parse::position& pos
                      )
        {
          s.emplace (read (pos));
        }
      }

      value_type read (util::parse::position& pos)
      {
        util::parse::require::skip_spaces (pos);

        if (pos.end())
        {
          throw util::parse::error::expected ("value", pos);
        }

        switch (*pos)
        {
        case '[':
          ++pos;
          util::parse::require::require (pos, ']');
          return we::type::literal::control();

        case 't':
          ++pos;
          util::parse::require::require (pos, "rue");
          return true;

        case 'f':
          ++pos;
          util::parse::require::require (pos, "alse");
          return false;

        case '\'':
          {
            ++pos;
            const char c (util::parse::require::plain_character (pos));
            util::parse::require::require (pos, '\'');
            return c;
          }

        case '"':
          ++pos;
          return util::parse::require::plain_string (pos, '\"');

        case '{':
          {
            ++pos;

            pnet::type::bitsetofint::type bs;

            util::parse::require::skip_spaces (pos);

            while (!pos.end() && *pos != '}')
            {
              bs.push_back (util::read_ulong (pos));

              util::parse::require::skip_spaces (pos);
            }

            util::parse::require::require (pos, '}');

            return std::move (bs);
          }

        case 'y':
          {
            ++pos;

            util::parse::require::require (pos, '(');

            we::type::bytearray ba;

            util::parse::require::skip_spaces (pos);

            while (!pos.end() && *pos != ')')
            {
              ba.push_back (util::read_ulong (pos));

              util::parse::require::skip_spaces (pos);
            }

            util::parse::require::require (pos, ')');

            return std::move (ba);
          }

        case 'M':
          {
            ++pos;
            util::parse::require::require (pos, "ap");

            std::map<value_type, value_type> m;

            util::parse::require::list
              ( pos
              , '[', ',', ']'
              , std::bind (map_item, std::ref (m), std::placeholders::_1)
              );

            return std::move (m);
          }

        case 'S':
          ++pos;
          if (pos.end())
          {
            throw util::parse::error::expected ("truct, et", pos);
          }
          else switch (*pos)
          {
          case 'e':
            {
              ++pos;
              util::parse::require::require (pos, 't');

              std::set<value_type> s;

              util::parse::require::list
                ( pos
                ,  '{', ',', '}'
                , std::bind (set_item, std::ref (s), std::placeholders::_1)
                );

              return std::move (s);
            }
          case 't':
            {
              ++pos;
              util::parse::require::require (pos, "ruct");

              structured_type m;

              util::parse::require::list
                ( pos
                ,  '[', ',', ']'
                , std::bind (struct_item, std::ref (m), std::placeholders::_1)
                );

              return std::move (m);
            }
          default:
            throw util::parse::error::expected ("truct, et", pos);
          }

        case 'L':
          {
            ++pos;
            util::parse::require::require (pos, "ist");

            std::list<value_type> l;

            util::parse::require::list
              ( pos
              ,  '(', ',', ')'
              , std::bind (list_item, std::ref (l), std::placeholders::_1)
              );

            return std::move (l);
          }

        case 's':
          {
            ++pos;
            util::parse::require::require (pos, "hared_");

            // Parse the cleanup place name (identifier)
            std::string cleanup_place;
            while (!pos.end() && (std::isalnum (*pos) || *pos == '_'))
            {
              cleanup_place += *pos;
              ++pos;
            }

            if (cleanup_place.empty())
            {
              throw util::parse::error::expected ("cleanup place name", pos);
            }

            util::parse::require::skip_spaces (pos);
            util::parse::require::require (pos, '(');
            util::parse::require::skip_spaces (pos);

            // Recursively parse any value type inside the shared
            value_type inner_value {read (pos)};

            util::parse::require::skip_spaces (pos);
            util::parse::require::require (pos, ')');

            return we::type::shared {std::move (inner_value), std::move (cleanup_place)};
          }

        default:
          return util::visit
            ( util::read_num (pos)
            , [] (auto const& n)
              {
                return value_type {n};
              }
            );
        }
      }

      value_type read (std::string const& input)
      {
        gspc::util::parse::position pos (input);

        return read (pos);
      }
    }
