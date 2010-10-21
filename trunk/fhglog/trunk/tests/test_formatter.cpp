/*
 * =====================================================================================
 *
 *       Filename:  test_formatter.cpp
 *
 *    Description:  Tests the formatting flags of the Formatter
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

#include <sstream> // std::ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/format.hpp>

static int test_format(const fhg::log::LogEvent &evt
              , const std::string &fmt
              , const std::string &expected
              , bool does_throw)
{
  using namespace fhg::log;
  std::clog << "** testing format string '" << fmt << "'...";

  if (! does_throw)
  {
    try
    {
      const std::string actual(format(fmt, evt));
      if (expected != actual)
      {
        std::clog << "FAILED!" << std::endl;
        std::clog << "\texpected: " << expected << std::endl;
        std::clog << "\tactual: " << actual << std::endl;
        return 1;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    catch (const std::exception &ex)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\texception: " << ex.what() << std::endl;
      return 1;
    }
    catch (...)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tunknown exception!" << std::endl;
      return 1;
    }
  }
  else
  {
    try {
      const std::string actual(format(fmt, evt));
      std::clog << "FAILED!" << std::endl;
      std::clog << "\texception expected!" << std::endl;
      std::clog << "\tactual: " << actual << std::endl;
    } catch(...) {
      std::clog << "OK!" << std::endl;
    }
  }
  return 0;
}

int main (int, char **)
{
  using namespace fhg::log;
  int errcount(0);

  errcount += test_format(FHGLOG_MKEVENT_HERE(TRACE, "hello"), "%s", "T", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(TRACE, "hello"), "%S", "TRACE", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(DEBUG, "hello"), "%s", "D", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(DEBUG, "hello"), "%S", "DEBUG", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(INFO, "hello"), "%s", "I", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(INFO, "hello"), "%S", "INFO", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(WARN, "hello"), "%s", "W", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(WARN, "hello"), "%S", "WARN", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(ERROR, "hello"), "%s", "E", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(ERROR, "hello"), "%S", "ERROR", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(FATAL, "hello"), "%s", "F", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(FATAL, "hello"), "%S", "FATAL", false);

  errcount += test_format(LogEvent(LogLevel::DEBUG, "tests/test_formatter.cpp", "main", __LINE__, "hello"), "%P", "tests/test_formatter.cpp", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(DEBUG, "hello"), "%p", "test_formatter.cpp", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(DEBUG, "hello"), "%L", FHGLOG_TOSTRING(__LINE__), false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(DEBUG, "hello"), "%l", "", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(DEBUG, "hello"), "%m", "hello", false);
  {
    std::ostringstream ostr;
    ostr << std::endl;
    errcount += test_format(FHGLOG_MKEVENT_HERE(DEBUG, "hello"), "%n", ostr.str(), false);
  }
  errcount += test_format(FHGLOG_MKEVENT_HERE(DEBUG, "hello"), "%%", "%", false);
  errcount += test_format(FHGLOG_MKEVENT_HERE(DEBUG, "hello"), "%U", "not-used", true);
  errcount += test_format(FHGLOG_MKEVENT_HERE(DEBUG, "hello"), "%", "not-used", true);

  return errcount;
}
