// alexander.petry@itwm.fraunhofer.de

#include "level.hpp"

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
      // TODO: make this better
      if      (name == "TRACE") lvl_ = TRACE;
      else if (name == "DEBUG") lvl_ = DEBUG;
      else if (name == "INFO")  lvl_ = INFO;
      else if (name == "WARN")  lvl_ = WARN;
      else if (name == "ERROR") lvl_ = ERROR;
      else if (name == "FATAL") lvl_ = ERROR;
      else throw std::runtime_error ("unknown log level: " + name);
    }
  }
}
