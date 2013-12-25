// alexander.petry@itwm.fraunhofer.de

#ifndef FHG_LOG_LOGLEVEL_INC
#define FHG_LOG_LOGLEVEL_INC

#include <iostream>
#include <stdexcept>
#include <string>

namespace fhg
{
  namespace log
  {
    class LogLevel
    {
    public:
      enum Level { TRACE
                 , DEBUG
                 , INFO
                 , WARN
                 , ERROR
                 , FATAL
                 };

      LogLevel (Level level)
        : lvl_ (level)
      {
        if (lvl_ < TRACE || lvl_ > FATAL)
        {
          throw std::runtime_error ("the specified log-level is out of range!");
        }
      }

      explicit LogLevel (const std::string&);

      const Level& lvl() const
      {
        return lvl_;
      }

    private:
      Level lvl_;
    };

    const std::string& string (LogLevel const&);
  }
}

#endif
