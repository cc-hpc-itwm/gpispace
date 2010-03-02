/*
 * =====================================================================================
 *
 *       Filename:  SyslogAppender.hpp
 *
 *    Description:  appends to syslog
 *
 *        Version:  1.0
 *        Created:  10/18/2009 01:23:04 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef FHG_LOG_SYSLOG_APPENDER_HPP
#define FHG_LOG_SYSLOG_APPENDER_HPP 1

#include <fhglog/fhglog-config.hpp>

#if !defined(HAVE_SYSLOG_H) || (HAVE_SYSLOG_H == 0)
#   error "I do not have syslog.h but SyslogAppender.hpp was requested."
#endif

#include <syslog.h>
#include <fhglog/Appender.hpp>

namespace fhg { namespace log {
  class SyslogAppender : public Appender
  {
  public:
    SyslogAppender(const std::string &ident, int options, int facility)
      : Appender(ident)
      , options_(options)
      , facility_(facility)
    {
      openlog(name().c_str(), options, facility); 
    }

    virtual ~SyslogAppender()
    {
      closelog();
    }
    
    void append(const fhg::log::LogEvent &) const;
  private:
    int options_;
    int facility_;
  };
}}

#endif
