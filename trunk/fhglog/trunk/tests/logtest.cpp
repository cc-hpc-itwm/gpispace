/*
 * =====================================================================================
 *
 *       Filename:  logtest.cpp
 *
 *    Description:  testing tool for the logging framework
 *
 *        Version:  1.0
 *        Created:  08/11/2009 11:26:47 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include    <fhglog/fhglog.hpp>
#include    <fhglog/Filter.hpp>
#include    <unistd.h>
#include    <sstream>

class Test
{
  public:
    Test() : log_(fhg::log::getLogger("test-class"))
    {
      LOG_DEBUG(log_, "Test class constructor");
    }
    ~Test()
    {
      LOG_DEBUG(log_, "Test class destructor");
    }
  private:
    fhg::log::LoggerApi log_;
};

std::string compute_large_output_string()
{
  sleep(3);
  return "computation took 3sec...: result 42";
}

int main(int argc, char **argv)
{
  using namespace fhg::log;
  LoggerApi logger(getLogger("test"));
  logger.setLevel(LogLevel::TRACE);
  if (argc > 1)
  {
    int lvl;
    std::istringstream istr(argv[1]);
    istr >> lvl;
    logger.setLevel(static_cast<LogLevel::Level>(lvl));
  }
  std::cerr << "the log-level has been set to: " << logger.getLevel().str() << "(" << logger.getLevel().lvl() << ")" << std::endl;

  logger.addAppender(Appender::ptr_t(new StreamAppender("console-long")))->setFormat(Formatter::DefaultFormatter());
  logger.addAppender(Appender::ptr_t(new StreamAppender("console-short")))->setFormat(Formatter::ShortFormatter());

  {
    Test test;
  }

  logger.log(LogEvent(LogLevel::TRACE
                            , __FILE__
                            , FHGLOG_FUNCTION
                            , __LINE__
                            , "trace message"));

  logger.log(LogEvent(LogLevel::DEBUG
                            , __FILE__
                            , FHGLOG_FUNCTION
                            , __LINE__
                            , "debug message"));

  logger.log(LogEvent(LogLevel::INFO
                            , __FILE__
                            , FHGLOG_FUNCTION
                            , __LINE__
                            , "info message"));

  logger.log(LogEvent(LogLevel::WARN
                            , __FILE__
                            , FHGLOG_FUNCTION
                            , __LINE__
                            , "warn message"));

  logger.log(LogEvent(LogLevel::ERROR
                            , __FILE__
                            , FHGLOG_FUNCTION
                            , __LINE__
                            , "error message"));
  LOG_ERROR(logger, "this is an error:" << 42);
  LOG_WARN(logger, "this is a warning:" << 42);

  LOG_TRACE(logger, "this message should not take up time, if it is not logged: " << compute_large_output_string());
}
