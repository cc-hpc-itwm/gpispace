#include "LogEvent.hpp"
#include <time.h>
#include <iostream>
#include <pthread.h>

using namespace fhg::log;

LogEvent::LogEvent(const severity_type &severity
                 , const file_type &file
                 , const function_type &function
                 , const line_type &line
                 , const message_type &message)
  : severity_(severity)
  , file_(file)
  , function_(function)
  , line_(line)
  , message_(message)
  , tstamp_(time(NULL))
  , thread_(static_cast<thread_type>(pthread_self()))
{
}

LogEvent::LogEvent(const LogEvent &e)
  : severity_(e.severity())
  , file_(e.file())
  , function_(e.function())
  , line_(e.line())
  , message_(e.message())
  , tstamp_(e.tstamp())
  , thread_(e.thread())
{
}

LogEvent::~LogEvent()
{
}

LogEvent &LogEvent::operator=(const LogEvent &e)
{
  if (this != &e) {
    severity_ = e.severity();
    file_ = e.file();
    function_ = e.function();
    line_ = e.line();
    message_ = e.message();
    tstamp_ = e.tstamp();
    thread_ = e.thread();
  }
  return *this;
}

bool LogEvent::operator==(const LogEvent &e) const
{
  if (this == &e) return true;
  if ( (file() == e.file())
    && (severity() == e.severity())
    && (function() == e.function())
    && (line() == e.line())
    && (message() == e.message())
    && (tstamp() == e.tstamp())
    && (thread() == e.thread()))
  {
    return true;
  }
  return false;
}

bool LogEvent::operator<(const LogEvent &rhs) const
{
  return tstamp() < rhs.tstamp();
}
