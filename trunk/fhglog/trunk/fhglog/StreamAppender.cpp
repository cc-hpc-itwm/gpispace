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
#include "format.hpp"
#include "fileno.hpp" // fileno for streams
#include "unistd.h"   // isatty

using namespace fhg::log;

static std::string colorControlCode (LogEvent::severity_type severity)
{
  switch (severity)
  {
  case LogEvent::severity_type::FATAL:
    return "\033[;1;31m"; // red
  case LogEvent::severity_type::ERROR:
    return "\033[;1;35m"; // magenta
  case LogEvent::severity_type::WARN:
    return "\033[;1;33m"; // yellow
  case LogEvent::severity_type::INFO:
    return "\033[;1;32m"; // green
  case LogEvent::severity_type::DEBUG:
    return "\033[;0m"; // normal
  case LogEvent::severity_type::TRACE:
    return "\033[;0m"; // normal
  default:
    return "\033[;0m"; // normal
  }
}

StreamAppender::StreamAppender( const std::string &a_name
                              , std::ostream &stream
                              , std::string const &fmt
                              , bool colored
                              )
  : Appender(a_name), stream_(stream), fmt_(fmt), colored_(colored)
{
  int fd (fileno (stream));
  if (fd < 0) colored_ = false;
  if (isatty(fd) != 1) colored_ = false;
}

void
StreamAppender::append(const LogEvent &evt)
{
  if (colored_)
    stream_ << colorControlCode (evt.severity());

  stream_ << format(fmt_, evt);

  if (colored_)
    stream_ << "\033[0m";
}		/* -----  end of method ConsoleAppender::append  ----- */
