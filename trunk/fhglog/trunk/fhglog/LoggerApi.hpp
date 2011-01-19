/*
 * =====================================================================================
 *
 *       Filename:  LoggerApi.hpp
 *
 *    Description:  The Logger interface
 *
 *        Version:  1.0
 *        Created:  09/13/2009 06:11:22 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef FHGLOG_LOGGER_API_HPP
#define FHGLOG_LOGGER_API_HPP 1

#include <string>
#include <fhglog/LogLevel.hpp>
#include <fhglog/Appender.hpp>
#include <fhglog/Logger.hpp>

namespace fhg { namespace log {

class LoggerApi;
typedef LoggerApi logger_t;
logger_t getLogger();
logger_t getLogger(const std::string &name);
logger_t getLogger(const std::string &name, const std::string & base);

#if FHGLOG_DISABLE_LOGGING != 1
  // forward declaration for the logger class
  typedef Logger::ptr_t logger_impl_t;

  class LoggerApi {
      friend logger_t getLogger();
      friend logger_t getLogger(const std::string &);
      friend logger_t getLogger(const std::string &, const std::string &);

    public:
      inline const std::string &name() const { return impl_->name(); }

      inline void setLevel(const LogLevel &level) { impl_->setLevel(level); }
      inline const LogLevel & getLevel() const { return impl_->getLevel(); }
      inline bool isLevelEnabled(const LogLevel &level) const { return impl_->isLevelEnabled(level); }

      inline void setFilter(const Filter::ptr_t &filter) { impl_->setFilter(filter); }
      inline const Filter::ptr_t &getFilter() const { return impl_->getFilter(); }
      inline bool isFiltered(const LogEvent &event) const { return impl_->isFiltered(event); }

      inline const Appender::ptr_t &addAppender(Appender::ptr_t appender) { return impl_->addAppender(appender); }
      inline const Appender::ptr_t &getAppender(const std::string &appender_name) const { return impl_->getAppender(appender_name); }
      inline void removeAppender(const std::string &appender_name) { impl_->removeAppender(appender_name); }
      inline void removeAllAppenders() { impl_->removeAllAppenders(); }

      inline void log(const LogEvent &event) { impl_->log(event); }
      inline void flush(void) { impl_->flush(); }
    private:
      explicit
      LoggerApi(logger_impl_t impl) : impl_(impl) {}

      logger_impl_t impl_;
  };

  inline logger_t getLogger()
  {
    return LoggerApi(Logger::get());
  }
  inline logger_t getLogger(const std::string &name)
  {
    return LoggerApi(Logger::get(name));
  }
  inline logger_t getLogger(const std::string &name, const std::string & base)
  {
    return LoggerApi(Logger::get(name, base));
  }

#else
  typedef char  logger_impl_t[sizeof(Logger::ptr_t)];

  class LoggerApi {
      friend logger_t getLogger();
      friend logger_t getLogger(const std::string &name);
      friend logger_t getLogger(const std::string &name, const std::string & base);
    public:
      const std::string &name() const { static std::string name_(""); return name_; }

      inline void setLevel(const LogLevel &) {}
      inline const LogLevel & getLevel() const { static LogLevel level_; return level_; }
      inline bool isLevelEnabled(const LogLevel &) const { return false; }

      inline void setFilter(const Filter::ptr_t &) { }
      inline const Filter::ptr_t &getFilter() const { static Filter::ptr_t filter(new NullFilter()); return filter; }
      inline bool isFiltered(const LogEvent &) const { return true; }

      inline const Appender::ptr_t &addAppender(const Appender::ptr_t &appender) { return appender; }
      inline const Appender::ptr_t &getAppender(const std::string &) { throw std::runtime_error("no such appender!"); }
      inline void removeAppender(const std::string &) {}
      inline void removeAllAppenders() { }

      inline void log(const LogEvent &) const {}
    private:
      explicit
      LoggerApi() {}

      logger_impl_t impl_;
  };

  inline logger_t getLogger()
  {
    return LoggerApi();
  }
  inline logger_t getLogger(const std::string &)
  {
    return LoggerApi();
  }
  inline logger_t getLogger(const std::string &, const std::string &)
  {
    return LoggerApi();
  }
#endif // FHGLOG_ENABLED

}}
#endif
