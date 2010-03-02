/*
 * =====================================================================================
 *
 *       Filename:  test_logging.cpp
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

#include <iostream>
#include    <sstream>
#include    <unistd.h>
#include    <fhglog/fhglog.hpp>
#include    <fhglog/Configuration.hpp>

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

static std::string compute_large_output_string()
{
  sleep(3);
  return "computation took 3sec...: result 42";
}

int main(int argc, char **argv)
{
  using namespace fhg::log;

  {
    fhg::log::Configurator::configure();
    getLogger().removeAllAppenders();
  }

  logger_t root(getLogger());
  logger_t logger(getLogger("test.mod"));
  logger.setLevel(LogLevel::TRACE);
  if (argc > 1)
  {
    int lvl;
    std::istringstream istr(argv[1]);
    istr >> lvl;
    logger.setLevel(static_cast<LogLevel::Level>(lvl));
  }
  std::cerr << "the log-level has been set to: " << logger.getLevel().str() << "(" << logger.getLevel().lvl() << ")" << std::endl;

  root.addAppender(Appender::ptr_t(new StreamAppender("console-long")))->setFormat(Formatter::Default());
  logger.addAppender(Appender::ptr_t(new StreamAppender("console-short")))->setFormat(Formatter::Full());

  LOG(INFO, "this is a very small info message");

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

  std::clog << "testing sizeof() of logger types...";
  if (sizeof(logger_t) != sizeof(logger_impl_t))
  {
    std::clog << "FAILED!" << std::endl;
    std::cerr << "\tsizeof(logger_t) = " << sizeof(logger_t) << std::endl;
    std::cerr << "\tsizeof(logger_impl_t) = " << sizeof(logger_impl_t) << std::endl;
  }
  else
  {
    std::clog << "OK" << std::endl;
  }
}
