#ifndef FHG_LOG_LOGGER_HPP
#define FHG_LOG_LOGGER_HPP 1

#include <string>

#include <fhglog/LogEvent.hpp>
#include <fhglog/Appender.hpp>

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
    bool operator==(const LogLevel &other) {
      return lvl_ == other.lvl_;
    }
    bool operator!=(const LogLevel &other) {
      return !(*this == other);
    }
    bool operator<(const LogLevel &other) {
      return lvl_ < other.lvl_;
    }
  private:
    Level lvl_;
  };

  class Logger {
    public:
      typedef std::size_t verbosity_type;

      void setLevel(const LogLevel &level);
      bool isLevelEnabled(const LogLevel &level);

      void log(const LogEvent &event);

      void addAppender(Appender::ptr_t appender);
      Appender::ptr_t getAppender(const std::string &appender_name);
      void removeAppender(const std::string &appender_name);
    private:
      Logger(const std::string &name);
      LogLevel lvl_;
      verbosity_type verbosity_;
  };
}}

#endif
