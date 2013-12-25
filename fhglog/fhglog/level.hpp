// alexander.petry@itwm.fraunhofer.de

#ifndef  FHG_LOG_LOGLEVEL_INC
#define  FHG_LOG_LOGLEVEL_INC

#include <iostream>
#include <string>
#include <stdexcept>

namespace fhg { namespace log {
  class LogLevel {
    public:
      enum Level {
          TRACE
        , DEBUG
        , INFO
        , WARN
        , ERROR
        , FATAL

        // keep the following definitions always up-to-date
        , MIN_LEVEL = TRACE
        , DEF_LEVEL = INFO
        , MAX_LEVEL = FATAL
      };

      LogLevel(Level level)
        : lvl_(level)
      {
        if (lvl_ < MIN_LEVEL || lvl_ > MAX_LEVEL)
        {
          throw std::runtime_error("the specified log-level is out of range!");
        }
      }

      explicit
      LogLevel(const std::string &level_name);

      const std::string &str() const;

      inline const Level &lvl() const { return lvl_; }
      operator Level () const { return lvl_; }

    private:
      Level lvl_;
  };
}}

inline std::ostream &operator<<(std::ostream &os, const fhg::log::LogLevel &lvl)
{
  os << lvl.str();
  return os;
}

#endif   /* ----- #ifndef FHG_LOG_LOGLEVEL_INC  ----- */
