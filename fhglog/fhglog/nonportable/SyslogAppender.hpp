// alexander.petry@itwm.fraunhofer.de

#ifndef FHG_LOG_SYSLOG_APPENDER_HPP
#define FHG_LOG_SYSLOG_APPENDER_HPP

#include <fhglog/fhglog-config.hpp>

#if !defined(HAVE_SYSLOG_H) || (HAVE_SYSLOG_H == 0)
#  error "I do not have syslog.h but SyslogAppender.hpp was requested."
#endif

#include <syslog.h>
#include <fhglog/Appender.hpp>

namespace fhg
{
  namespace log
  {
    class SyslogAppender : public Appender
    {
    public:
      SyslogAppender ( const std::string& ident
                     , const std::string& fmt
                     , int options
                     , int facility
                     )
        : Appender (ident)
        , _fmt (fmt)
        , _options (options)
        , _facility (facility)
      {
        openlog(name().c_str(), _options, _facility);
      }

      virtual ~SyslogAppender()
      {
        closelog();
      }

      void append (const fhg::log::LogEvent&);

    private:
      std::string _fmt;
      int _options;
      int _facility;
    };
  }
}

#endif
