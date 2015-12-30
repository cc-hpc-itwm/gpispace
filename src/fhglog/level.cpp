// mirko.rahn@itwm.fraunhofer.de

#include <fhglog/level.hpp>
#include <fhglog/level_io.hpp>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/from_string.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

#include <stdexcept>

namespace fhg
{
  namespace log
  {
    const std::string& string (Level l)
    {
      if (l < TRACE || l > ERROR)
      {
        throw std::runtime_error ("the specified log-level is out of range!");
      }

      static std::string const map[] =
        { "TRACE"
        , "INFO"
        , "WARN"
        , "ERROR"
        };

      return map[l];
    }

    namespace
    {
      Level from_parse_position (fhg::util::parse::position& pos)
      {
        namespace parse = fhg::util::parse;

        std::string const any
          ("one of 'TRACE', 'INFO', 'WARN' or 'ERROR'");

        if (pos.end())
        {
          throw parse::error::expected (any, pos);
        }

        switch (*pos)
        {
        case 'T': ++pos; parse::require::require (pos, "RACE"); return TRACE;
        case 'I': ++pos; parse::require::require (pos, "NFO"); return INFO;
        case 'W': ++pos; parse::require::require (pos, "ARN"); return WARN;
        case 'E': ++pos; parse::require::require (pos, "RROR"); return ERROR;
        default: throw parse::error::expected (any, pos);
        }
      }
    }

    Level from_string (std::string const& name)
    {
      return fhg::util::parse::from_string<Level> (&from_parse_position, name);
    }
  }
}
