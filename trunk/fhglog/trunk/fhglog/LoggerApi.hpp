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

// FIXME: move this to a configuration header
#define FHGLOG_ENABLED 1

#if FHGLOG_ENABLED == 1
#include <fhglog/Logger.hpp>
  // forward declaration for the logger class
  typedef Logger logger_impl_t;

  class LoggerApi {
    public:
      explicit
      LoggerApi(logger_impl_t::ptr_t impl) : impl_(impl) {}

      inline const std::string &name() const { return impl_->name(); }
      inline void setLevel(const LogLevel &level) { impl_->setLevel(level); }
      inline const LogLevel & getLevel() const { return impl_->getLevel(); }
      inline bool isLevelEnabled(const LogLevel &level) const { return impl_->isLevelEnabled(level); }
      inline void log(const LogEvent &event) const { impl_->log(event); }
      inline const Appender::ptr_t &addAppender(Appender::ptr_t appender) { return impl_->addAppender(appender); }
      inline const Appender::ptr_t &getAppender(const std::string &appender_name) const { return impl_->getAppender(appender_name); }
      inline void removeAppender(const std::string &appender_name) { impl_->removeAppender(appender_name); }
    private:
      logger_impl_t::ptr_t impl_;
  };
  typedef LoggerApi logger_t;

  inline logger_t getLogger(const std::string &name)
  {
    return LoggerApi(Logger::get(name));
  }
#else
  typedef void    logger_impl_t;

  class LoggerApi {
    public:
      explicit
      LoggerApi(logger_impl_t *impl) : impl_(impl) {}

      const std::string &name() const { static std::string name_(""); return name_; }
      void setLevel(const LogLevel &level) {}
      const LogLevel & getLevel() const { static LogLevel level_(LogLevel::UNSET); return level_; }
      bool isLevelEnabled(const LogLevel &level) const { return false; }
      void log(const LogEvent &event) const {}
      const Appender::ptr_t &addAppender(const Appender::ptr_t &appender) { return appender; }
      const Appender::ptr_t &getAppender(const std::string &appender_name) { throw std::runtime_error("no such appender!"); }
      void removeAppender(const std::string &appender_name) {}
    private:
      logger_impl_t *impl_;
  };

  typedef LoggerApi logger_t;
  inline logger_t getLogger(const std::string &name)
  {
    return LoggerApi(NULL);
  }
#endif // FHGLOG_ENABLED
}}
#endif
