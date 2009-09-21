/*
 * =====================================================================================
 *
 *       Filename:  test_appender.cpp
 *
 *    Description:  Tests the appender for the fhglog logger
 *
 *        Version:  1.0
 *        Created:  09/18/2009 04:38:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sstream> // ostringstream
#include <cstdlib> // exit
#include <fhglog/fhglog.hpp>

///#define FHGLOG_MKEVENT(var, level, message) ::fhg::log::LogEvent var(::fhg::log::LogLevel::level, __FILE__, FHGLOG_FUNCTION, __LINE__, message)
int main (int argc, char **argv)
{
  int errcount(0);

  using namespace fhg::log;

  std::ostringstream logstream;
  logger_t log(getLogger());
  log.addAppender(Appender::ptr_t(new StreamAppender("stringstream", logstream)))->setFormat(Formatter::Custom("%m"));

  // test 1 - append a single event
  {
    std::clog << "** testing event appending...";
    FHGLOG_MKEVENT(evt, DEBUG, "hello world!");
    log.log(evt);
    if (logstream.str() != "hello world!")
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tlogged message: " << logstream.str() << std::endl;
      std::clog << "\texpected: " << "hello world!" << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
  }

  std::exit(errcount);
}
