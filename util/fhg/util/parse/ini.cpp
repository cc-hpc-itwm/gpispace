// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/parse/ini.hpp>

#include <fhg/util/parse/from_string.hpp>
#include <fhg/util/parse/require.hpp>
#include <fhg/util/parse/until.hpp>

#include <boost/foreach.hpp>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      namespace
      {
        bool is_end_of_header_label (position const& pos)
        {
          return isspace (*pos) || *pos == ']';
        }
        bool is_end_of_key (position const& pos)
        {
          return isspace (*pos) || *pos == '=';
        }
        bool is_end_of_value (position const& pos)
        {
          return *pos == '\n';
        }

        std::string require_header (position& pos)
        {
          require::require (pos, '[');
          require::skip_spaces (pos);

          std::string const label
            (until (pos, &is_end_of_header_label));

          require::skip_spaces (pos);

          std::string sublabel;

          if (!pos.end() && *pos != ']')
          {
            sublabel = "." + require::string (pos);

            require::skip_spaces (pos);
          }

          require::require (pos, ']');

          return label + sublabel;
        }

        void skip_comment (position& pos)
        {
          while (*pos == '#' || *pos == ';')
          {
            ++pos;

            while (!pos.end() && *pos != '\n')
            {
              ++pos;
            }

            require::skip_spaces (pos);
          }
        }

        std::map<std::string, std::string> ini (position& pos)
        {
          std::map<std::string, std::string> m;

          require::skip_spaces (pos);
          skip_comment (pos);

          while (!pos.end())
          {
            std::string const header (require_header (pos));
            require::skip_spaces (pos);

            while (!pos.end() && *pos != '[')
            {
              std::string const key (until (pos, &is_end_of_key));
              require::skip_spaces (pos);
              require::require (pos, '=');
              require::skip_spaces (pos);

              m[header + "." + key] = (!pos.end() && *pos == '"')
                ? require::string (pos) : until (pos, &is_end_of_value);

              require::skip_spaces (pos);
              skip_comment (pos);
            }
          }

          return m;
        }
      }

      std::map<std::string, std::string> ini_map (std::string const& input)
      {
        return fhg::util::parse::from_string
          <std::map<std::string, std::string> > (&ini, input);
      }
    }
  }
}
