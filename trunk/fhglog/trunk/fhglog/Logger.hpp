#ifndef FHG_LOG_LOGGER_HPP
#define FHG_LOG_LOGGER_HPP 1

#include <map>
#include <list>
#include <string>

#include <fhglog/LogLevel.hpp>
#include <fhglog/LogEvent.hpp>
#include <fhglog/Appender.hpp>

/**
  Common logging framework.

  LoggerApi root_logger(Logger::get());
  LoggerApi my_logger(Logger::get("module"));

  my_logger.log(LogEvent(...));
*/
namespace fhg { namespace log {
  /*
   * This class wraps around a simple Logger object.
   *
   * The main reason to have a separate loggerapi class and an implementation
   * is the following, when we want to disable logging completely, we should
   * make sure that the classes using loggers as member variables still have
   * the same size. In this case, sizeof(LoggerApi) == sizeof(void*) - that
   * means in the disabled case, we can just allocate an empty void pointer.
   *
   */
  class Logger {
    public:
      typedef std::size_t verbosity_type;

      static Logger &get(const std::string &name);
      ~Logger() {}

      const std::string &name() const;
      void setLevel(const LogLevel &level);
      const LogLevel &getLevel() const;
      bool isLevelEnabled(const LogLevel &level) const;

      void log(const LogEvent &event) const;
      Appender::ptr_t addAppender(Appender::ptr_t appender);
      Appender::ptr_t getAppender(const std::string &appender_name) const;
      void removeAppender(const std::string &appender_name);
    private:
      static Logger& getRootLogger();

      explicit
      Logger(const std::string &name);

      std::string name_;
      LogLevel lvl_;
      verbosity_type verbosity_;

      typedef std::map<std::string, Logger*> logger_map_t;
      logger_map_t loggers_;

      typedef std::list<Appender::ptr_t> appender_list_t;
      appender_list_t appenders_;
  };
}}

#endif
