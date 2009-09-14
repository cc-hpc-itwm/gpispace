#ifndef FHG_LOG_LOGEVENT_HPP
#define FHG_LOG_LOGEVENT_HPP 1

#include <string>
#include <sys/types.h>
#include <pthread.h>
#include <fhglog/LogLevel.hpp>

namespace fhg { namespace log {
  class LogEvent {
    public:
      typedef LogLevel severity_type;
      typedef std::string file_type;
      typedef std::string function_type;
      typedef std::size_t line_type;
      typedef std::string message_type;
      typedef long long tstamp_type;
      typedef pid_t     pid_type;
      typedef pthread_t tid_type;

      static std::string &severityToString(const severity_type &severity);

      LogEvent(const severity_type &severity
             , const file_type &path
             , const function_type &function
             , const line_type &line
             , const message_type &message);

      LogEvent(const LogEvent &);

      ~LogEvent();

      LogEvent &operator=(const LogEvent &);
      bool operator==(const LogEvent &) const;
      bool operator!=(const LogEvent &rhs) const
      {
        return !(*this == rhs);
      }

      bool operator<(const LogEvent &) const;

      inline const severity_type &severity() const { return severity_; }
      inline const file_type &file() const { return file_; }
      inline const file_type &path() const { return path_; }
      inline const function_type &function() const { return function_; }
      inline const line_type &line() const { return line_; }
      inline const message_type &message() const { return message_; }
      inline const tstamp_type &tstamp() const { return tstamp_; }
      inline const pid_type &pid() const { return pid_; }
      inline const tid_type &tid() const { return tid_; }
    private:
      std::string get_filename_from_path(const std::string &path) const;

      severity_type severity_;
      file_type path_;
      file_type file_;
      function_type function_;
      line_type line_;
      message_type message_;
      tstamp_type tstamp_;
      pid_type pid_;
      tid_type tid_;
  };
}}

#endif
