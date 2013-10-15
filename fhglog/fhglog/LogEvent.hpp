#ifndef FHG_LOG_LOGEVENT_HPP
#define FHG_LOG_LOGEVENT_HPP 1

#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <sys/types.h>
#include <fhglog/LogLevel.hpp>

#include <boost/cstdint.hpp>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      class position;
    }
  }
}

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

      LogEvent(util::parse::position&);
      static LogEvent from_string (const std::string&);
      LogEvent();
      LogEvent(const severity_type &severity
             , const file_type &pa
             , const function_type &function
             , const line_type &line
             , const message_type &message);

      bool operator<(const LogEvent &) const;

      const std::size_t& id() const { return id_; }
      const severity_type& severity() const { return severity_; }
      const file_type& path() const { return path_; }
      const function_type& function() const { return function_; }
      const line_type& line() const { return line_; }
      const message_type& message() const { return message_; }
      const tstamp_type& tstamp() const { return tstamp_; }
      const pid_type& pid() const { return pid_; }
      const tid_type& tid() const { return tid_; }
      const trace_type& trace() const { return trace_; }
      const std::string& host() const { return host_; }
      const tags_type& tags () const { return tags_; }

      std::size_t &id() { return id_; }
      severity_type &severity() { return severity_; }
      file_type &path() { return path_; }
      function_type &function() { return function_; }
      line_type &line() { return line_; }
      message_type &message() { return message_; }
      tstamp_type &tstamp() { return tstamp_; }
      pid_type &pid() { return pid_; }
      tid_type &tid() { return tid_; }
      trace_type &trace() { return trace_; }
      std::string &host() { return host_; }

      void trace (const std::string &name) const
      {
        trace_.push_back (name);
      }
      void tag (const std::string &t)
      {
        tags_.insert (t);
      }
      void untag (const std::string &t)
      {
        tags_.erase (t);
      }

      std::string encoded() const;

    private:
      uint64_t id_;
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
      tags_type tags_;
  };
}}

std::ostream& operator<< (std::ostream&, fhg::log::LogEvent const&);

#endif
