/*
 * =====================================================================================
 *
 *       Filename:  test_disabled_logging.cpp
 *
 *    Description:  Tests log-event creation
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
#include <iostream>
#include <cstdlib> // exit
#include <ctime>   // time
#include <unistd.h> // sleep

#include <fhglog/fhglog.hpp>

int main (int, char **)
{
  using namespace fhg::log;

  int errcount(0);

  {
    std::clog << "** testing event creation...";
    LogEvent evt(LogLevel::DEBUG, "tests/test_event.cpp", "main", 34, "hello world!");
    if (evt.severity().lvl() != LogLevel::DEBUG)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tactual severity: " << evt.severity().lvl() << std::endl;
      std::clog << "\texpected: " << LogLevel::DEBUG << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
  }

  {
    std::clog << "** testing illegal loglevel...";
    try {
      LogEvent evt(static_cast<LogLevel::Level>(LogLevel::MAX_LEVEL+1)
                 , "tests/test_event.cpp"
                 , "main"
                 , 34
                 , "hello world!");

      std::clog << "FAILED!" << std::endl;
      std::clog << "\tloglevel was out of range, exception expected!" << std::endl;
      ++errcount;
    } catch (const std::exception &ex) {
      std::clog << "OK!" << std::endl;
    }
  }

  std::exit(errcount);
}
