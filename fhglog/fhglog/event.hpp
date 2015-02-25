#pragma once

#include <fhglog/level.hpp>

#include <sys/types.h>

#include <sstream>
#include <string>
#include <vector>

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

namespace fhg
{
  namespace log
  {
    class LogEvent
    {
    public:
      typedef std::string file_type;
      typedef std::string function_type;
      typedef std::size_t line_type;
      typedef std::string message_type;

      LogEvent (util::parse::position&);
      static LogEvent from_string (const std::string&);

      LogEvent();
      LogEvent ( const Level& severity
               , const file_type& pa
               , const function_type& function
               , const line_type& line
               , const message_type& message
               );

      bool operator< (const LogEvent&) const;

      const Level& severity() const { return severity_; }
      const file_type& path() const { return path_; }
      const function_type& function() const { return function_; }
      const line_type& line() const { return line_; }
      const message_type& message() const { return message_; }
      const double& tstamp() const { return tstamp_; }
      const pid_t& pid() const { return pid_; }
      const pid_t& tid() const { return tid_; }
      const std::string& host() const { return host_; }

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
    };
  }
}

std::ostream& operator<< (std::ostream&, fhg::log::LogEvent const&);
