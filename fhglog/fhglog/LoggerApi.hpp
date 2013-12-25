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
#include <fhglog/level.hpp>
#include <fhglog/Appender.hpp>
#include <fhglog/Logger.hpp>

namespace fhg { namespace log {

class LoggerApi;
typedef LoggerApi logger_t;
logger_t getLogger();
logger_t getLogger(const std::string &name);
logger_t getLogger(const std::string &name, const std::string & base);

  // forward declaration for the logger class
  typedef Logger::ptr_t logger_impl_t;

  class LoggerApi {
      friend logger_t getLogger();
      friend logger_t getLogger(const std::string &);
      friend logger_t getLogger(const std::string &, const std::string &);

    public:
      inline const std::string &name() const { return impl_->name(); }

      inline void setLevel(const LogLevel &level) { impl_->setLevel(level); }
      inline bool isLevelEnabled(const LogLevel &level) const { return impl_->isLevelEnabled(level); }

      inline void setFilter(const Filter::ptr_t &filter) { impl_->setFilter(filter); }
      inline bool isFiltered(const LogEvent &event) const { return impl_->isFiltered(event); }

      inline const Appender::ptr_t &addAppender(Appender::ptr_t appender) { return impl_->addAppender(appender); }

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


}}
#endif
