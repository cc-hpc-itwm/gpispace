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
      LogEvent (util::parse::position&);
      static LogEvent from_string (const std::string&);

      LogEvent();
      LogEvent (const Level& severity, std::string const& message);

      bool operator< (const LogEvent&) const;

      const Level& severity() const { return severity_; }
      const std::string& message() const { return message_; }
      const double& tstamp() const { return tstamp_; }
      const pid_t& pid() const { return pid_; }
      const pid_t& tid() const { return tid_; }
      const std::string& host() const { return host_; }

      std::string encoded() const;

    private:
      Level severity_;
      std::string message_;
      double tstamp_;
      pid_t pid_;
      pid_t tid_;
      std::string host_;
    };
  }
}

std::ostream& operator<< (std::ostream&, fhg::log::LogEvent const&);
