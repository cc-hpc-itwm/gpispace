#ifndef FHG_LOG_LOGEVENT_HPP
#define FHG_LOG_LOGEVENT_HPP 1

#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <sys/types.h>
#include <fhglog/LogLevel.hpp>

namespace fhg { namespace log {
  class LogEvent {
    public:
      typedef LogLevel severity_type;
      typedef std::string file_type;
      typedef std::string function_type;
      typedef std::size_t line_type;
      typedef std::string message_type;
      typedef std::string module_type;
      typedef unsigned long long tstamp_type;
      typedef pid_t         pid_type;
      typedef unsigned long tid_type;
      typedef std::vector<std::string> trace_type;
      typedef std::set<std::string> tags_type;

      LogEvent();
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

      inline       std::size_t id () const { return id_; }
      inline const severity_type &severity() const { return severity_; }
      inline const file_type &path() const { return path_; }
      inline const function_type &function() const { return function_; }
      inline const line_type &line() const { return line_; }
      inline const message_type &message() const { return message_; }
      inline const tstamp_type &tstamp() const { return tstamp_; }
      inline const pid_type &pid() const { return pid_; }
      inline const tid_type &tid() const { return tid_; }
      inline const trace_type &trace() const { return trace_; }
      inline const std::string &host() const { return host_; }
      inline const tags_type &tags () const { return tags_; }

      inline       std::size_t &id () { return id_; }
      inline severity_type &severity() { return severity_; }
      inline file_type &path() { return path_; }
      inline function_type &function() { return function_; }
      inline line_type &line() { return line_; }
      inline message_type &message() { return message_; }
      inline tstamp_type &tstamp() { return tstamp_; }
      inline pid_type &pid() { return pid_; }
      inline tid_type &tid() { return tid_; }
      inline trace_type &trace() { return trace_; }
      inline std::string &host() { return host_; }

      inline void trace (const std::string &name) const
      {
        trace_.push_back (name);
      }

      inline void tag (const std::string &t) const
      {
        tags_.insert (t);
      }

      inline void untag (const std::string &t)
      {
        tags_.erase (t);
      }

      inline void finish() const
      {
        if (message_.empty())
        {
          const_cast<std::string&>(message_) = message_buffer_.str();
        }
      }
      inline std::ostream &stream() { return message_buffer_; }

    std::ostream & encode (std::ostream &, int flags = 0) const;
    std::istream & decode (std::istream &);

    private:
      std::size_t id_;
      severity_type severity_;
      file_type path_;
      function_type function_;
      line_type line_;
      message_type message_;
      tstamp_type tstamp_;
      pid_type pid_;
      tid_type tid_;
      std::string host_;
      mutable trace_type trace_;
      mutable tags_type tags_;
      std::ostringstream message_buffer_;
  };
}}

std::ostream & operator << (std::ostream &os, fhg::log::LogEvent const &evt);
std::istream & operator >> (std::istream &is, fhg::log::LogEvent &evt);

#endif
