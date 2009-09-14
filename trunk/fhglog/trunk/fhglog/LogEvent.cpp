#include "LogEvent.hpp"
#include <time.h>
#include <iostream>
#include <pthread.h>

using namespace fhg::log;

LogEvent::LogEvent(const severity_type &severity
                 , const std::string &logged_via
                 , const file_type &path
                 , const function_type &function
                 , const line_type &line
                 , const message_type &message)
  : severity_(severity)
  , logged_via_(logged_via)
  , path_(path)
  , file_(get_filename_from_path(path))
  , function_(function)
  , line_(line)
  , message_(message)
  , tstamp_(time(NULL))
  , pid_(getpid())
  , tid_(static_cast<tid_type>(pthread_self()))
{
}

LogEvent::LogEvent(const LogEvent &e)
  : severity_(e.severity())
  , logged_via_(e.logged_via())
  , path_(e.path())
  , file_(e.file())
  , function_(e.function())
  , line_(e.line())
  , message_(e.message())
  , tstamp_(e.tstamp())
  , pid_(e.pid())
  , tid_(e.tid())
{
}

LogEvent::~LogEvent()
{
}

LogEvent &LogEvent::operator=(const LogEvent &e)
{
  if (this != &e) {
    severity_ = e.severity();
    path_ = e.path();
    file_ = e.file();
    function_ = e.function();
    line_ = e.line();
    message_ = e.message();
    tstamp_ = e.tstamp();
    pid_ = e.pid();
    tid_ = e.tid();
  }
  return *this;
}

bool LogEvent::operator==(const LogEvent &e) const
{
  if (this == &e) return true;
  if (
       (file() == e.file())
    && (path() == e.path())
    && (severity() == e.severity())
    && (function() == e.function())
    && (line() == e.line())
    && (message() == e.message())
    && (tstamp() == e.tstamp())
    && (pid() == e.pid())
    && (tid() == e.tid())
    )
  {
    return true;
  }
  return false;
}

bool LogEvent::operator<(const LogEvent &rhs) const
{
  return tstamp() < rhs.tstamp();
}

std::string LogEvent::get_filename_from_path(const std::string &path) const
{
  // TODO: the following should be coded with boost::filesystem due to platform
  // independence
  static const std::string path_sep("/");
  std::string::size_type last_path_segment_idx = path.find_last_of(path_sep.c_str());
  if (last_path_segment_idx == std::string::npos)
  {
    // return the whole path since we could not find a separator
    return path;
  }
  else
  {
    // slit the path at that position and return the remaining part
    return path.substr(last_path_segment_idx+1);
  }
}
