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
      LoggerApi(logger_impl_t *impl) : impl_(impl) {}

      const std::string &name() const;
      void setLevel(const LogLevel &level);
      const LogLevel & getLevel() const;
      bool isLevelEnabled(const LogLevel &level) const;
      void log(const LogEvent &event) const;
      Appender::ptr_t addAppender(Appender::ptr_t appender);
      Appender::ptr_t getAppender(const std::string &appender_name) const;
      void removeAppender(const std::string &appender_name);
    private:
      logger_impl_t *impl_;
  };

  inline LoggerApi getLogger(const std::string &name)
  {
    return LoggerApi(&Logger::get(name));
  }
#else
  typedef void   logger_impl_t;

  class LoggerApi {
    public:
      explicit
      LoggerApi(logger_impl_t *impl) : impl_(impl) {}

      const std::string &name() const { return ""; }
      void setLevel(const LogLevel &level) {}
      const LogLevel & getLevel() const { return LogLevel::UNSET; }
      bool isLevelEnabled(const LogLevel &level) { return false; }
      void log(const LogEvent &event) const {}
      Appender::ptr_t addAppender(Appender::ptr_t appender) { return appender; }
      Appender::ptr_t getAppender(const std::string &appender_name) { return Appender::ptr_t((Appender*)(0)); }
      void removeAppender(const std::string &appender_name) {}
    private:
      logger_impl_t *impl_;
  };

  inline LoggerApi getLogger(const std::string &name)
  {
    return LoggerApi(NULL);
  }
#endif
}}
#endif
