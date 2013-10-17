// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/util/parse/require.hpp>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/position.hpp>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      namespace require
      {
        void require (position& pos, const char& what)
        {
          if (pos.end() || *pos != what)
          {
            throw error::expected (std::string (1, what), pos);
          }

          ++pos;
        }
        void require (position& pos, const std::string& what)
        {
          std::string::const_iterator what_pos (what.begin());
          const std::string::const_iterator what_end (what.end());

          while (what_pos != what_end)
          {
            if (pos.end() || *pos != *what_pos)
            {
              throw error::expected (std::string (what_pos, what_end), pos);
            }
            else
            {
              ++pos;
              ++what_pos;
            }
          }
        }

        void skip_spaces (position& pos)
        {
          while (!pos.end() && isspace (*pos))
          {
            ++pos;
          }
        }

        char plain_character (position& pos)
        {
          if (pos.end())
          {
            throw error::expected ("character", pos);
          }

          char const c (*pos);

          ++pos;

          return c;
        }

        std::string plain_string
          (position& pos, const char until, const char escape)
        {
          std::string s;

          while (!pos.end() && *pos != until)
          {
            if (*pos == escape)
            {
              ++pos;

              if (!pos.end() && (*pos == until || *pos == escape))
              {
                s.push_back (*pos);

                ++pos;
              }
              else
              {
                throw error::expected ( std::string (1, until)
                                      + " or "
                                      + std::string (1, escape)
                                      , pos
                                      );
              }
            }
            else
            {
              s.push_back (*pos);

              ++pos;
            }
          }

          if (pos.end())
          {
            throw error::expected (std::string (1, until), pos);
          }

          ++pos;

          return s;
        }

        std::string identifier (position& pos)
        {
          std::string id;

          if (pos.end() || !(isalpha (*pos) || *pos == '_'))
          {
            throw error::expected ("identifier [a-zA-Z_][a-zA-Z_0-9]*", pos);
          }

          id.push_back (*pos); ++pos;

          while (!pos.end() && (isalpha (*pos) || *pos == '_' || isdigit (*pos)))
          {
            id.push_back (*pos); ++pos;
          }

          return id;
        }

        void token (position& pos, const std::string& what)
        {
          skip_spaces (pos);
          require::require (pos, what);
        }

        char character (position& pos)
        {
          skip_spaces (pos);
          require::require (pos, '\'');
          const char ch (plain_character (pos));
          require::require (pos, '\'');
          return ch;
        }

        std::string string (position& pos)
        {
          skip_spaces (pos);
          require::require (pos, '"');
          return plain_string (pos, '"');
        }

        bool boolean (position& pos)
        {
          skip_spaces (pos);

          if ( pos.end() || ( *pos != '0' && *pos != '1' && *pos != 'f' && *pos != 'n'
                            && *pos != 'o' && *pos != 't' && *pos != 'y'
                            )
             )
          {
            throw error::expected
              ("0' or '1' or 'false' or 'no' or 'off' or 'on' or 'true' or 'yes", pos);
          }

          switch (*pos)
          {
          case '0':
            ++pos;
            return false;

          case '1':
            ++pos;
            return true;

          case 'f':
            ++pos;
            require::require (pos, "alse");
            return false;

          case 'n':
            ++pos;
            require::require (pos, 'o');
            return false;

          case 'o':
            ++pos;

            if (pos.end() || (*pos != 'f' && *pos != 'n'))
            {
              throw error::expected ("ff' or 'n", pos);
            }

            switch (*pos)
            {
            case 'f':
              ++pos;
              require::require (pos, 'f');
              return false;

            case 'n':
              ++pos;
              return true;
            }

          case 't':
            ++pos;
            require::require (pos, "rue");
            return true;

          case 'y':
            ++pos;
            require::require (pos, "es");
            return true;

          default:
            return false; // never happens, as throw above, but compilers warn.
          }
        }

        void list ( position& pos
                  , const char open, const char sep, const char close
                  , const boost::function<void (position&)>& f
                  , const bool skip_space_before_element
                  , const bool skip_space_after_element
                  )
        {
          skip_spaces (pos);

          require::require (pos, open);

          if (skip_space_before_element)
          {
            skip_spaces (pos);
          }

          bool closed (false);
          bool expect_sep (false);

          do
          {
            if (pos.end())
            {
              throw error::expected
                ( std::string (1, close)
                + " or "
                + (expect_sep ? std::string (1, sep) : "<list_element>")
                , pos
                );
            }

            if (*pos == close)
            {
              ++pos;

              closed = true;
            }
            else if (expect_sep)
            {
              require::require (pos, sep);

              if (skip_space_before_element)
              {
                skip_spaces (pos);
              }

              expect_sep = false;
            }
            else if (*pos == sep)
            {
              throw error::expected ("<list_element>", pos);
            }
            else
            {
              f (pos);

              if (skip_space_after_element)
              {
                skip_spaces (pos);
              }

              expect_sep = true;
            }
          }
          while (!closed);
        }

        std::string rest (position& pos)
        {
          std::string ret;
          for (; !pos.end(); ++pos)
          {
            ret.push_back (*pos);
          }
          return ret;
        }
      }
    }
  }
}
