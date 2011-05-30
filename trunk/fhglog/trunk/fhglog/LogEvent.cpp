#include "LogEvent.hpp"
#include <sys/time.h>
#include <iostream>
#include "util.hpp"

using namespace fhg::log;

static LogEvent::tstamp_type now()
{
//  struct timeval tv;
//  gettimeofday (&tv, NULL);
//  return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
  return time(NULL);
}

LogEvent::LogEvent(const severity_type &a_severity
                 , const file_type &a_path
                 , const function_type &a_function
                 , const line_type &a_line
                 , const message_type &a_message)
  : severity_(a_severity)
  , path_(a_path)
  , file_(get_filename_from_path(a_path))
  , function_(a_function)
  , line_(a_line)
  , message_(a_message)
  , tstamp_(now())
  , pid_(getpid())
  , tid_(gettid())
  , module_(get_module_name_from_path(a_path))
{
}

LogEvent::LogEvent()
  : severity_()
  , path_()
  , file_()
  , function_()
  , line_()
  , message_()
  , tstamp_()
  , pid_()
  , tid_()
{
}

LogEvent::LogEvent(const LogEvent &e)
  : severity_(e.severity())
  , path_(e.path())
  , file_(e.file())
  , function_(e.function())
  , line_(e.line())
  , message_(e.message())
  , tstamp_(e.tstamp())
  , pid_(e.pid())
  , tid_(e.tid())
  , logged_via_(e.logged_via())
  , logged_on_(e.logged_on())
  , module_(e.module())
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
    logged_via_ = e.logged_via();
    logged_on_ = e.logged_on();
    module_ = e.module();
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
    && (logged_via() == e.logged_via())
    && (module() == e.module())
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
