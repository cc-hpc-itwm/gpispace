/*
 * =====================================================================================
 *
 *       Filename:  StreamAppender.cpp
 *
 *    Description:  appending to a std::ostream
 *
 *        Version:  1.0
 *        Created:  08/25/2009 11:02:12 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include    "StreamAppender.hpp"

using namespace fhg::log;

std::string StreamAppender::colorControlCode (LogEvent::severity_type severity) const
{
  switch (severity)
  {
  case LogEvent::severity_type::FATAL:
    return "\033[;1;31m";
  case LogEvent::severity_type::ERROR:
    return "\033[;1;31m";
  case LogEvent::severity_type::WARN:
    return "\033[;1;34m";
  case LogEvent::severity_type::INFO:
    return "\033[;2;33m";
  case LogEvent::severity_type::DEBUG:
    return "\033[;2;32m";
  case LogEvent::severity_type::TRACE:
    return "\033[;2;32m";
  default:
    return "\033[;0m";
  }
}

void
StreamAppender::append(const LogEvent &evt)
{
  if (colored_)
    stream_ << colorControlCode (evt.severity());
  stream_ << getFormat()->format(evt);
}		/* -----  end of method ConsoleAppender::append  ----- */
