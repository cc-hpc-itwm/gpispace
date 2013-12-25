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
    const std::string& LogLevel::str() const
    {
      static std::string LevelToStringMap_[] =
        { "TRACE"
        , "DEBUG"
        , "INFO"
        , "WARN"
        , "ERROR"
        , "FATAL"
        };

      assert (lvl_ >= 0 && lvl_ < 6);

      return LevelToStringMap_[lvl_];
    }

    LogLevel::LogLevel (const std::string& name)
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
      case 'T': ++pos; parse::require::require (pos, "RACE"); lvl_ = TRACE; break;
      case 'D': ++pos; parse::require::require (pos, "EBUG"); lvl_ = DEBUG; break;
      case 'I': ++pos; parse::require::require (pos, "NFO"); lvl_ = INFO; break;
      case 'W': ++pos; parse::require::require (pos, "ARN"); lvl_ = WARN; break;
      case 'E': ++pos; parse::require::require (pos, "RROR"); lvl_ = ERROR; break;
      case 'F': ++pos; parse::require::require (pos, "ATAL"); lvl_ = FATAL; break;
      default:
        throw parse::error::expected (any, pos);
      }

      if (!pos.end())
      {
        throw parse::error::expected (any, pos);
      }
    }
  }
}
