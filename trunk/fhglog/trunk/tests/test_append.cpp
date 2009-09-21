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
#include <fhglog/NullAppender.hpp>

int main (int argc, char **argv)
{
  using namespace fhg::log;

  int errcount(0);
  logger_t log(getLogger());

  {
    std::clog << "** testing adding removing appender...";
    log.addAppender(Appender::ptr_t(new NullAppender("null")));
    try {
      log.getAppender("null");
    } catch(...) {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tappender has not been added!" << std::endl;
      ++errcount;
    }
    log.removeAppender("null");
    try {
      log.getAppender("null");
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tappender has not been removed correctly!" << std::endl;
      ++errcount;
    } catch (...) {
      std::clog << "OK!" << std::endl;
    }
  }

  std::ostringstream logstream;
  log.addAppender(Appender::ptr_t(new StreamAppender("stringstream", logstream)))->setFormat(Formatter::Custom("%m"));

  {
    std::clog << "** testing event appending (one appender)...";
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
    logstream.str("");
  }

  {
    std::clog << "** testing event appending (two appender)...";
    std::ostringstream logstream2;
    log.addAppender(Appender::ptr_t(new StreamAppender("stringstream-2", logstream2)))->setFormat(Formatter::Custom("%m"));
    FHGLOG_MKEVENT(evt, DEBUG, "hello world!");
    log.log(evt);
    if (logstream.str() != "hello world!" || logstream2.str() != "hello world!")
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
    log.removeAppender("stringstream-2");
    logstream.str("");
  }

  std::exit(errcount);
}
