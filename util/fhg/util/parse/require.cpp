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

        void token (fhg::util::parse::position& pos, const std::string& what)
        {
          pos.skip_spaces();
          pos.require (what);
        }

        char character (fhg::util::parse::position& pos)
        {
          pos.skip_spaces();
          pos.require ("'");
          const char ch (plain_character (pos));
          pos.require ("'");
          return ch;
        }

        std::string string (fhg::util::parse::position& pos)
        {
          token (pos, "\"");
          return pos.until ('"');
        }

        bool boolean (fhg::util::parse::position& pos)
        {
          pos.skip_spaces();

          if ( pos.end() || ( *pos != '0' && *pos != '1' && *pos != 'f' && *pos != 'n'
                            && *pos != 'o' && *pos != 't' && *pos != 'y'
                            )
             )
          {
            throw fhg::util::parse::error::expected
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
            pos.require ("alse");
            return false;

          case 'n':
            ++pos;
            pos.require ("o");
            return false;

          case 'o':
            ++pos;

            if (pos.end() || (*pos != 'f' && *pos != 'n'))
            {
              throw fhg::util::parse::error::expected ("ff' or 'n", pos);
            }

            switch (*pos)
            {
            case 'f':
              ++pos;
              pos.require ("f");
              return false;

            case 'n':
              ++pos;
              return true;
            }

          case 't':
            ++pos;
            pos.require ("rue");
            return true;

          case 'y':
            ++pos;
            pos.require ("es");
            return true;

          default:
            return false; // never happens, as throw above, but compilers warn.
          }
        }
      }
    }
  }
}
