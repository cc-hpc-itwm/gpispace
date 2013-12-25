// alexander.petry@itwm.fraunhofer.de

#include "level.hpp"

#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>
#include <fhg/util/parse/error.hpp>

#include <cassert>

namespace fhg
{
  namespace log
  {
    const std::string& string (LogLevel const& l)
    {
      static std::string const map[] =
        { "TRACE"
        , "DEBUG"
        , "INFO"
        , "WARN"
        , "ERROR"
        , "FATAL"
        };

      assert (l.lvl() >= 0 && l.lvl() < 6);

      return map[l.lvl()];
    }

    namespace
    {
      LogLevel::Level parse_level (std::string const& name)
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
        case 'T': ++pos; parse::require::require (pos, "RACE"); return LogLevel::TRACE;
        case 'D': ++pos; parse::require::require (pos, "EBUG"); return LogLevel::DEBUG;
        case 'I': ++pos; parse::require::require (pos, "NFO"); return LogLevel::INFO;
        case 'W': ++pos; parse::require::require (pos, "ARN"); return LogLevel::WARN;
        case 'E': ++pos; parse::require::require (pos, "RROR"); return LogLevel::ERROR;
        case 'F': ++pos; parse::require::require (pos, "ATAL"); return LogLevel::FATAL;
        default: throw parse::error::expected (any, pos);
        }
      }
    }

    LogLevel::LogLevel (const std::string& name)
      : lvl_ (parse_level (name))
    {}
  }
}
