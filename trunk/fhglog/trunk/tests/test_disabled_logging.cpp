/*
 * =====================================================================================
 *
 *       Filename:  test_disabled_logging.cpp
 *
 *    Description:  Tests the logging framework when logging has been disabled
 *
 *        Version:  1.0
 *        Created:  09/21/2009 04:46:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sstream> // ostringstream
#include <ctime>   // time
#include <unistd.h> // sleep

#define FHGLOG_DISABLE_LOGGING 1
#include <fhglog/fhglog.hpp>

// UNUSED
// static std::string simulate_computation(std::size_t time = 5)
// {
//   sleep(time);
//   return "";
// }

int main (int, char **)
{
  using namespace fhg::log;

  int errcount(0);
  logger_t log(getLogger());

  std::ostringstream logstream;
  log.addAppender(Appender::ptr_t(new StreamAppender("stringstream", logstream)))->setFormat(Formatter::Custom("%m"));

  {
    std::clog << "** testing manual event appending...";
    log.log(FHGLOG_MKEVENT_HERE(DEBUG, "hello world!"));
    if (! logstream.str().empty())
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tlogged message: " << logstream.str() << std::endl;
      std::clog << "\tempty string expected" << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
    logstream.str("");
  }

  {
    std::clog << "** testing TRACE macro...";
    std::time_t start(time(NULL));
    LOG_TRACE(log, simulate_computation(10));
    std::time_t end(time(NULL));
    if ((end -start) > 1)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tlog string should not be built!" << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
  }

  {
    std::clog << "** testing DEBUG macro...";
    std::time_t start(time(NULL));
    LOG_DEBUG(log, simulate_computation(10));
    std::time_t end(time(NULL));
    if ((end -start) > 1)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tlog string should not be built!" << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
  }

  {
    std::clog << "** testing INFO macro...";
    std::time_t start(time(NULL));
    LOG_INFO(log, simulate_computation(10));
    std::time_t end(time(NULL));
    if ((end -start) > 1)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tlog string should not be built!" << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
  }

  {
    std::clog << "** testing WARN macro...";
    std::time_t start(time(NULL));
    LOG_WARN(log, simulate_computation(10));
    std::time_t end(time(NULL));
    if ((end -start) > 1)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tlog string should not be built!" << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
  }

  {
    std::clog << "** testing ERROR macro...";
    std::time_t start(time(NULL));
    LOG_ERROR(log, simulate_computation(10));
    std::time_t end(time(NULL));
    if ((end -start) > 1)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tlog string should not be built!" << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
  }

  {
    std::clog << "** testing FATAL macro...";
    std::time_t start(time(NULL));
    LOG_FATAL(log, simulate_computation(10));
    std::time_t end(time(NULL));
    if ((end -start) > 1)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tlog string should not be built!" << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
  }
  return errcount;
}
