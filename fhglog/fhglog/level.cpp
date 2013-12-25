// mirko.rahn@itwm.fraunhofer.de

#include "level.hpp"

#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>
#include <fhg/util/parse/error.hpp>

#include <stdexcept>

#include <cassert>

namespace fhg
{
  namespace log
  {
    Level from_int (int l)
    {
      if (l < TRACE || l > FATAL)
      {
        throw std::runtime_error ("the specified log-level is out of range!");
      }

      return Level (l);
    }

    const std::string& string (Level l)
    {
      static std::string const map[] =
        { "TRACE"
        , "DEBUG"
        , "INFO"
        , "WARN"
        , "ERROR"
        , "FATAL"
        };

      assert (l >= 0 && l < 6);

      return map[l];
    }

    Level from_string (std::string const& name)
    {
      namespace parse = fhg::util::parse;

      std::string const any
        ("one of 'TRACE', 'DEBUG', 'INFO', 'WARN', 'ERROR' or 'FATAL'");

      parse::position_string pos (name);

      if (pos.end())
      {
        throw parse::error::expected (any, pos);
      }

      switch (*pos)
      {
      case 'T': ++pos; parse::require::require (pos, "RACE"); return TRACE;
      case 'D': ++pos; parse::require::require (pos, "EBUG"); return DEBUG;
      case 'I': ++pos; parse::require::require (pos, "NFO"); return INFO;
      case 'W': ++pos; parse::require::require (pos, "ARN"); return WARN;
      case 'E': ++pos; parse::require::require (pos, "RROR"); return ERROR;
      case 'F': ++pos; parse::require::require (pos, "ATAL"); return FATAL;
      default: throw parse::error::expected (any, pos);
      }
    }
  }
}
