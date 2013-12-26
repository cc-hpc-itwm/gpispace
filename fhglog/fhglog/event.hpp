#ifndef FHG_LOG_LOGEVENT_HPP
#define FHG_LOG_LOGEVENT_HPP 1

#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <sys/types.h>
#include <fhglog/level.hpp>

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
      typedef std::string file_type;
      typedef std::string function_type;
      typedef std::size_t line_type;
      typedef std::string message_type;

      LogEvent(util::parse::position&);
      static LogEvent from_string (const std::string&);
      LogEvent();
      LogEvent(const Level &severity
             , const file_type &pa
             , const function_type &function
             , const line_type &line
             , const message_type &message
             , std::vector<std::string> const& = std::vector<std::string>()
             );

      bool operator<(const LogEvent &) const;

      const Level& severity() const { return severity_; }
      const file_type& path() const { return path_; }
      const function_type& function() const { return function_; }
      const line_type& line() const { return line_; }
      const message_type& message() const { return message_; }
      const double& tstamp() const { return tstamp_; }
      const pid_t& pid() const { return pid_; }
      const pid_t& tid() const { return tid_; }
      const std::vector<std::string>& trace() const { return trace_; }
      const std::string& host() const { return host_; }
      const std::vector<std::string>& tags () const { return tags_; }

      void trace (const std::string &name) const
      {
        trace_.push_back (name);
      }

      std::string encoded() const;

    private:
      Level severity_;
      file_type path_;
      function_type function_;
      line_type line_;
      message_type message_;
      double tstamp_;
      pid_t pid_;
      pid_t tid_;
      std::string host_;
      mutable std::vector<std::string> trace_;
      std::vector<std::string> tags_;
  };
}}

std::ostream& operator<< (std::ostream&, fhg::log::LogEvent const&);

#endif
