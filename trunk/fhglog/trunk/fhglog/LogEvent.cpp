#include "LogEvent.hpp"
#include <time.h>
#include <iostream>

using namespace fhg::log;

LogEvent::LogEvent(const file_type &file
                 , const function_type &function
                 , const line_type &line
                 , const message_type &message)
  : file_(file)
  , function_(function)
  , line_(line)
  , message_(message)
  , tstamp_(time(NULL))
  , thread_(0)
{
  std::clog << "FIXME: thread identification is missing" << std::endl;
}

LogEvent::LogEvent(const LogEvent &e)
  : file_(e.file())
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
    file_ = e.file();
    function_ = e.function();
    line_ = e.line();
    message_ = e.message();
    tstamp_ = e.tstamp();
    thread_ = e.thread();
  }
  return *this;
}

bool LogEvent::operator==(const LogEvent &e)
{
  if (this == &e) return true;
  if ( (file() == e.file())
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

bool LogEvent::operator<(const LogEvent &rhs)
{
  return tstamp() < rhs.tstamp();
}
