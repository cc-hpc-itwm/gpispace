/*
 * =====================================================================================
 *
 *       Filename:  LogLevel.hpp
 *
 *    Description:  defines available severity levels
 *
 *        Version:  1.0
 *        Created:  08/11/2009 05:24:16 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef  FHG_LOG_LOGLEVEL_INC
#define  FHG_LOG_LOGLEVEL_INC

#include <string>

namespace fhg { namespace log {
  class LogLevel {
    public:
      enum Level {
        UNSET = 0,
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
      };

      LogLevel(Level lvl)
        : lvl_(lvl) {}
      LogLevel(const LogLevel &other)
        : lvl_(other.lvl_) {}

      LogLevel &operator=(const LogLevel &other) {
        if (this != &other) {
          lvl_ = other.lvl_;
        }
        return *this;
      }
      bool operator==(const LogLevel &other) const {
        return lvl_ == other.lvl_;
      }
      bool operator!=(const LogLevel &other) const {
        return !(*this == other);
      }
      bool operator<(const LogLevel &other) const {
        return lvl_ < other.lvl_;
      }
      bool operator<=(const LogLevel &other) const {
        return lvl_ == other.lvl_ || lvl_ < other.lvl_;
      }

      const std::string &str() const;
    private:
      Level lvl_;
  };
}}
#endif   /* ----- #ifndef FHG_LOG_LOGLEVEL_INC  ----- */
