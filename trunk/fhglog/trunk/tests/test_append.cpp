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

class FormattingNullAppender : public fhg::log::Appender
{
  public:
    FormattingNullAppender(const std::string &a_name) : fhg::log::Appender(a_name) {}

    void append(const fhg::log::LogEvent &evt) const
    {
      fhg::log::Appender::getFormat()->format(evt);
    }
};

int main (int , char **)
{
  using namespace fhg::log;

  int errcount(0);
  logger_t log(getLogger());

  {
    std::clog << "** testing adding and removing appender...";
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

  {
    std::clog << "** testing adding and removing all appender...";
    log.addAppender(Appender::ptr_t(new NullAppender("null-0")));
    log.addAppender(Appender::ptr_t(new NullAppender("null-1")));
    log.addAppender(Appender::ptr_t(new NullAppender("null-2")));
    log.addAppender(Appender::ptr_t(new NullAppender("null-3")));
    try {
      log.getAppender("null-0");
      log.getAppender("null-1");
      log.getAppender("null-2");
      log.getAppender("null-3");
    } catch(...) {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tappender has not been added!" << std::endl;
      ++errcount;
    }
    log.removeAllAppenders();
    try {
      log.getAppender("null-0");
      log.getAppender("null-1");
      log.getAppender("null-2");
      log.getAppender("null-3");
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tappender has not been removed correctly!" << std::endl;
      ++errcount;
    } catch (...) {
      std::clog << "OK!" << std::endl;
    }
  }

  {
    std::clog << "** testing formatting performance...";
    log.addAppender(Appender::ptr_t(new FormattingNullAppender("null")))->setFormat(Formatter::Full());
    for (std::size_t count(0); count < 1000000; ++count)
    {
      log.log(FHGLOG_MKEVENT_HERE(DEBUG, "hello world!"));
    }
    std::clog << "OK!" << std::endl;
    log.removeAppender("null");
  }


  std::ostringstream logstream;
  log.addAppender(Appender::ptr_t(new StreamAppender("stringstream", logstream)))->setFormat(Formatter::Custom("%m"));

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
