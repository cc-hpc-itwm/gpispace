#ifndef FHG_LOG_LOGGER_HPP
#define FHG_LOG_LOGGER_HPP 1

#include <list>
#include <string>

#include <fhglog/memory.hpp>
#include <fhglog/level.hpp>
#include <fhglog/event.hpp>
#include <fhglog/Appender.hpp>
#include <fhglog/Filter.hpp>

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
      typedef shared_ptr<Logger> ptr_t;

      static Logger::ptr_t get();
      static Logger::ptr_t get(const std::string &name, const std::string &base = "default");

      explicit
      Logger(const std::string &name);
      Logger(const std::string &name, const Logger &inherit_from);

      const std::string &name() const;
      const std::string &parent() const;
      void setLevel(const LogLevel &level);
      void setFilter(const Filter::ptr_t &filter);
      const Filter::ptr_t &getFilter() const;
      const LogLevel &getLevel() const;
      inline bool isLevelEnabled(const LogLevel &level) const
      {
        return level >= lvl_;
      }

      inline bool isFiltered(const LogEvent &evt) const
      {
        return (*filter_)(evt);
      }

      void log(const LogEvent &event);
      void flush();

      const Appender::ptr_t &addAppender(const Appender::ptr_t &appender);
      void removeAppender(const Appender::ptr_t&);
      void removeAllAppenders();

    private:
      std::string name_;
      LogLevel lvl_;
      Filter::ptr_t filter_;

      typedef std::list<Appender::ptr_t> appender_list_t;
      appender_list_t appenders_;
  };

    void terminate ();
}}

#endif
