/*
 * =====================================================================================
 *
 *       Filename:  test_synch_appender.cpp
 *
 *    Description:  Tests the synchronous appender for the fhglog logger
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
#include <fhglog/fhglog.hpp>
#include <fhglog/SynchronizedAppender.hpp>
#include <fhglog/NullAppender.hpp>

int main (int , char **)
{
  using namespace fhg::log;

  int errcount(0);
  logger_t log(getLogger());

  std::ostringstream logstream;
  log.addAppender(Appender::ptr_t(new SynchronizedAppender(new StreamAppender("stringstream", logstream))))->setFormat(Formatter::Custom("%m"));

  {
    std::clog << "** testing event appending (one appender)...";
    log.log(FHGLOG_MKEVENT_HERE(DEBUG, "hello world!"));
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

  return errcount;
}
